/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.cpp
*/

#include "Core.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include "../graphics/ResourceManager.hpp"
#include "../graphics/Window.hpp"

struct InputMapping
{
    std::string action;
    CLIENT::KeyCode keyCode;
};

static const std::vector<struct InputMapping> INPUT_MAPPINGS = {
    {"MOVE_DOWN", CLIENT::KeyCode::DOWN},
    {"MOVE_UP", CLIENT::KeyCode::UP},
    {"MOVE_LEFT", CLIENT::KeyCode::LEFT},
    {"MOVE_RIGHT", CLIENT::KeyCode::RIGHT},
    {"SHOOT", CLIENT::KeyCode::SHOOT}};

/**
 * @brief Constructs a Core client instance and initializes the game.
 *
 * @param argv Command line arguments passed to the program.
 *
 * @details
 * The constructor performs the following steps:
 * - Parses command line arguments for hostname (-h) and port (-p).
 * - Prompts the user to enter a username; defaults to "Player" if empty.
 * - Initializes the keybind manager and loads key bindings from "keybinds.cfg".
 * - Initializes network communication and registers callbacks.
 * - Loads necessary game resources.
 *
 * @throws CoreError If required command line arguments are missing or invalid.
 */
CLIENT::Core::Core(char** argv)
    : _port(0),
      _running(false),
      _myPlayerId(255),
      _hasNewSnapshot(false),
      _gameState(GameState::PLAYING),
      _defeatTextureLoaded(false)
{
    parseCommandLineArgs(argv);

    std::cout << "Enter username: ";
    std::getline(std::cin, _username);
    if (_username.empty())
        _username = "Player";

    _keybindManager = std::make_unique<KeybindManager>();
    _keybindManager->loadFromFile("keybinds.cfg");

    initializeNetwork();
    loadResources();
}

/**
 * @brief Destructs the Core client instance, stopping network communication.
 *
 * @details
 * - Stops the running flag.
 * - Waits for the network thread to finish if it is joinable.
 */
CLIENT::Core::~Core()
{
    _running = false;
    if (_networkThread.joinable())
        _networkThread.join();
}

/**
 * @brief Parses command line arguments to set hostname and port.
 *
 * @param argv Command line arguments.
 *
 * @throws CoreError If required arguments (-h or -p) are missing or invalid.
 *
 * @details
 * Recognized arguments:
 * - `-h <hostname>`: Specifies the server hostname.
 * - `-p <port>`: Specifies the server port.
 *
 * Any missing or invalid arguments will result in a CoreError exception.
 */
void CLIENT::Core::parseCommandLineArgs(char** argv)
{
    for (int i = 1; argv[i]; ++i) {
        std::string arg = argv[i];
        if (arg == "-p") {
            if (!argv[i + 1])
                throw CoreError("Missing value for -p");
            _port = std::stoi(argv[++i]);
        } else if (arg == "-h") {
            if (!argv[i + 1])
                throw CoreError("Missing value for -h");
            _hostname = argv[++i];
        }
    }

    if (_port == 0 || _hostname.empty())
        throw CoreError("Missing -p or -h argument");
}

/**
 * @brief Initializes network client, sets up callbacks, and sends join request.
 *
 * @details
 * - Creates a NetworkClient instance with the hostname and port.
 * - Registers callbacks to handle incoming player IDs, events, snapshots,
 * timeouts, and kills.
 * - Sends a join request to the server with the current username.
 * - Starts receiving data from the server.
 */
void CLIENT::Core::initializeNetwork()
{
    _networkClient = std::make_unique<NetworkClient>(_hostname, _port);
    setupNetworkCallbacks();
    _networkClient->sendJoin(_username);
    _networkClient->startReceiving();

    std::cout << "[Core] Client initialized with username: " << _username
              << "\n";
    std::cout << "[Core] Waiting for player ID from server...\n";
}

/**
 * @brief Sets up all network event callbacks for the client.
 *
 * @details
 * The following callbacks are registered:
 * - `setOnPlayerIdReceived`: Calls `handlePlayerIdReceived` when the server
 * assigns a player ID.
 * - `setOnPlayerEvent`: Calls `handlePlayerEvent` on player events.
 * - `setOnSnapshot`: Calls `handleSnapshotReceived` when receiving game state
 * snapshots.
 * - `setOnTimeout`: Pushes a timeout message to `_incomingMessages`.
 * - `setOnKilled`: Pushes a killed message to `_incomingMessages`.
 *
 * @note Uses mutex `_incomingMutex` to safely push messages to the queue.
 */
void CLIENT::Core::setupNetworkCallbacks()
{
    _networkClient->setOnPlayerIdReceived(
        [this](uint8_t playerId) { handlePlayerIdReceived(playerId); });

    _networkClient->setOnPlayerEvent(
        [this](uint8_t playerId, uint8_t eventType) {
            handlePlayerEvent(playerId, eventType);
        });

    _networkClient->setOnSnapshot([this](const std::vector<uint8_t>& payload) {
        handleSnapshotReceived(payload);
    });
    _networkClient->setOnTimeout([this](uint8_t playerId) {
        std::lock_guard<std::mutex> lock(_incomingMutex);
        _incomingMessages.push("TIMEOUT:" + std::to_string(playerId));
    });

    _networkClient->setOnKilled([this](uint8_t playerId) {
        std::lock_guard<std::mutex> lock(_incomingMutex);
        _incomingMessages.push("KILLED:" + std::to_string(playerId));
    });
}

/**
 * @brief Handles the reception of the player's assigned ID from the server.
 *
 * @param playerId The player ID assigned by the server.
 *
 * @details
 * Updates the internal `_myPlayerId` variable, prints a log message,
 * and pushes a "PLAYER_ID" message into the incoming message queue.
 *
 * @note Uses `_incomingMutex` to safely push messages to the queue.
 */
void CLIENT::Core::handlePlayerIdReceived(uint8_t playerId)
{
    _myPlayerId = playerId;
    std::cout << "*** [Network Callback] Assigned Player ID: "
              << int(_myPlayerId) << " ***\n";

    std::lock_guard<std::mutex> lock(_incomingMutex);
    _incomingMessages.push("PLAYER_ID:" + std::to_string(playerId));
}

/**
 * @brief Handles player join or leave events from the server.
 *
 * @param playerId The ID of the player who joined or left.
 * @param eventType 0 for join, 1 for leave.
 *
 * @details
 * Pushes either a "PLAYER_JOIN" or "PLAYER_LEAVE" message into the
 * incoming message queue based on the event type.
 *
 * @note Thread-safe via `_incomingMutex`.
 */
void CLIENT::Core::handlePlayerEvent(uint8_t playerId, uint8_t eventType)
{
    std::lock_guard<std::mutex> lock(_incomingMutex);
    if (eventType == 0) {
        _incomingMessages.push("PLAYER_JOIN:" + std::to_string(playerId));
    } else if (eventType == 1) {
        _incomingMessages.push("PLAYER_LEAVE:" + std::to_string(playerId));
    }
}

/**
 * @brief Handles the reception of a game state snapshot from the server.
 *
 * @param payload A vector of bytes representing the serialized snapshot.
 *
 * @details
 * Stores the snapshot in `_pendingSnapshot` and sets `_hasNewSnapshot`
 * to true. Thread-safe via `_snapshotMutex`.
 */
void CLIENT::Core::handleSnapshotReceived(const std::vector<uint8_t>& payload)
{
    std::lock_guard<std::mutex> lock(_snapshotMutex);
    _pendingSnapshot = payload;
    _hasNewSnapshot = true;
}

/**
 * @brief Loads all game resources including textures and background music.
 *
 * @details
 * Calls the following functions internally:
 * - `loadGameTextures()`
 * - `loadParallaxTextures()`
 * - `loadBackgroundMusic()`
 */
void CLIENT::Core::loadResources()
{
    loadGameTextures();
    loadParallaxTextures();
    loadBackgroundMusic();
    std::cout << "Resources loaded\n";
}

/**
 * @brief Loads the main game textures into the ResourceManager.
 */
void CLIENT::Core::loadGameTextures()
{
    auto& rm = ResourceManager::getInstance();

    rm.loadTexture(
        "assets/sprites/r-typesheet42.png", "assets/sprites/r-typesheet42.png");
    rm.loadTexture(
        "assets/sprites/playerProjectiles.png",
        "assets/sprites/playerProjectiles.png");
    rm.loadTexture(
        "assets/sprites/r-typesheet5.png", "assets/sprites/r-typesheet5.png");
    rm.loadTexture(
        "assets/sprites/r-typesheet9.png", "assets/sprites/r-typesheet9.png");
    rm.loadTexture(
        "assets/sprites/r-typesheet10.png", "assets/sprites/r-typesheet10.png");
    rm.loadTexture(
        "assets/sprites/r-typesheet11.png", "assets/sprites/r-typesheet11.png");
    rm.loadTexture(
        "assets/sprites/r-typesheet30a.png",
        "assets/sprites/r-typesheet30a.png");
    rm.loadTexture("assets/sprites/birds.png", "assets/sprites/birds.png");
    rm.loadTexture(
        "assets/sprites/coloredpipes.png", "assets/sprites/coloredpipes.png");
}

/**
 * @brief Loads parallax background textures into the ResourceManager.
 */
void CLIENT::Core::loadParallaxTextures()
{
    auto& rm = ResourceManager::getInstance();

    rm.loadTexture(
        "assets/sprites/parallax/1.png", "assets/sprites/parallax/1.png");
    rm.loadTexture(
        "assets/sprites/parallax/2.png", "assets/sprites/parallax/2.png");
    rm.loadTexture(
        "assets/sprites/parallax/3.png", "assets/sprites/parallax/3.png");
    rm.loadTexture(
        "assets/sprites/parallax/4.png", "assets/sprites/parallax/4.png");
}

/**
 * @brief Loads and starts the background music.
 *
 * @details
 * Opens the "backgroundmusic.wav" file, sets looping and volume, and
 * starts playback. Logs an error if the music cannot be loaded.
 */
void CLIENT::Core::loadBackgroundMusic()
{
    _backgroundMusic = std::make_unique<sf::Music>();
    if (!_backgroundMusic->openFromFile("assets/sound/backgroundmusic.wav")) {
        std::cerr << "Failed to load background music\n";
    } else {
        _backgroundMusic->setLooping(true);
        _backgroundMusic->setVolume(100);
        _backgroundMusic->play();
    }
}

/**
 * @brief Starts the main game loop including graphics and network threads.
 */
void CLIENT::Core::run()
{
    _running = true;
    _networkThread = std::thread(&Core::networkLoop, this);
    graphicsLoop();
    _running = false;
    if (_networkThread.joinable())
        _networkThread.join();
}

/**
 * @brief The main network thread loop for sending outgoing messages.
 */
void CLIENT::Core::networkLoop()
{
    std::cout << "[Network Thread] Started\n";

    while (_running) {
        processOutgoingMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "[Network Thread] Stopped\n";
}

/**
 * @brief Processes all outgoing messages queued for the server.
 *
 * @details
 * Currently handles messages that start with "INPUT:" and sends them
 * via `sendInputMessage()`.
 */
void CLIENT::Core::processOutgoingMessages()
{
    std::lock_guard<std::mutex> lock(_outgoingMutex);

    while (!_outgoingMessages.empty()) {
        std::string msg = _outgoingMessages.front();
        _outgoingMessages.pop();

        if (msg.find("INPUT:") == 0) {
            sendInputMessage(msg);
        }
    }
}

/**
 * @brief Sends an input action message to the server.
 *
 * @param msg The message string in the format "INPUT:<keyCode>:<action>".
 *
 * @details
 * Extracts the key code and action from the message and sends it using the
 * `_networkClient`. Does nothing if the player ID has not been assigned yet.
 */
void CLIENT::Core::sendInputMessage(const std::string& msg)
{
    if (_myPlayerId == 255) {
        std::cout << "[Network] Cannot send input, no player ID assigned yet\n";
        return;
    }

    size_t pos1 = msg.find(':', 6);
    uint8_t keyCode = std::stoi(msg.substr(6, pos1 - 6));
    uint8_t action = std::stoi(msg.substr(pos1 + 1));

    _networkClient->sendInput(_myPlayerId, keyCode, action);
}

/**
 * @brief Reads a 4-byte float from a serialized payload.
 *
 * @param payload The byte vector containing the serialized float.
 * @param offset The current read offset; will be incremented by 4.
 * @return The deserialized float value, or 0.0f if out of bounds.
 *
 * @details
 * Converts 4 consecutive bytes starting at offset into a float using
 * big-endian format.
 */
float CLIENT::Core::readFloat(
    const std::vector<uint8_t>& payload, size_t& offset)
{
    if (offset + 4 > payload.size())
        return 0.0f;

    uint32_t temp = (static_cast<uint32_t>(payload[offset]) << 24) |
                    (static_cast<uint32_t>(payload[offset + 1]) << 16) |
                    (static_cast<uint32_t>(payload[offset + 2]) << 8) |
                    static_cast<uint32_t>(payload[offset + 3]);
    offset += 4;

    float result;
    std::memcpy(&result, &temp, sizeof(float));
    return result;
}

/**
 * @brief Parses a game snapshot payload to update active entities.
 *
 * @param payload A byte vector representing the serialized game state snapshot.
 *
 * @details
 * Iterates through the payload and calls `parseSnapshotEntity` for each entity.
 * Deactivates entities that are not present in the current snapshot.
 */
void CLIENT::Core::parseSnapshot(const std::vector<uint8_t>& payload)
{
    if (payload.empty())
        return;

    size_t offset = 0;
    std::set<uint8_t> activeEntitiesInSnapshot;

    while (offset < payload.size()) {
        if (!parseSnapshotEntity(payload, offset, activeEntitiesInSnapshot))
            break;
    }

    _entityManager->deactivateEntitiesNotInSet(activeEntitiesInSnapshot);
}

/**
 * @brief Parses a single entity from a snapshot payload.
 *
 * @param payload The snapshot byte vector.
 * @param offset Reference to the current read position in the payload.
 * @param activeEntities Set of currently active entity IDs to update.
 * @return True if the entity was successfully parsed; false otherwise.
 */
bool CLIENT::Core::parseSnapshotEntity(
    const std::vector<uint8_t>& payload, size_t& offset,
    std::set<uint8_t>& activeEntities)
{
    if (offset >= payload.size())
        return false;

    uint8_t entityId = payload[offset++];

    if (offset + 8 > payload.size())
        return false;
    float x = readFloat(payload, offset);
    float y = readFloat(payload, offset);

    if (offset >= payload.size())
        return false;
    uint8_t pathLen = payload[offset++];

    if (offset + pathLen > payload.size())
        return false;
    std::string spritePath(
        payload.begin() + offset, payload.begin() + offset + pathLen);
    offset += pathLen;

    if (offset + 16 > payload.size())
        return false;
    float rectPosX = readFloat(payload, offset);
    float rectPosY = readFloat(payload, offset);
    float rectSizeX = readFloat(payload, offset);
    float rectSizeY = readFloat(payload, offset);

    activeEntities.insert(entityId);
    updateOrCreateEntity(
        entityId, x, y, spritePath, rectPosX, rectPosY, rectSizeX, rectSizeY);

    return true;
}

/**
 * @brief Updates an existing entity or creates a new one if it does not exist.
 *
 * @param entityId The unique ID of the entity.
 * @param x X-coordinate of the entity.
 * @param y Y-coordinate of the entity.
 * @param spritePath Path to the entity's sprite texture.
 * @param rectPosX X-coordinate of the texture rect.
 * @param rectPosY Y-coordinate of the texture rect.
 * @param rectSizeX Width of the texture rect.
 * @param rectSizeY Height of the texture rect.
 */
void CLIENT::Core::updateOrCreateEntity(
    uint8_t entityId, float x, float y, const std::string& spritePath,
    float rectPosX, float rectPosY, float rectSizeX, float rectSizeY)
{
    GameEntity* entity = _entityManager->getEntity(entityId);
    bool needsNewSprite = false;

    if (entity) {
        if (!entity->currentSpritePath.empty() &&
            entity->currentSpritePath != spritePath) {
            needsNewSprite = true;
        }
    } else {
        _entityManager->createSimpleEntity(entityId);
        entity = _entityManager->getEntity(entityId);
        needsNewSprite = true;
    }

    if (entity) {
        updateEntityPosition(entity, x, y);
        updateEntitySprite(
            entity, entityId, spritePath, needsNewSprite, rectPosX, rectPosY,
            rectSizeX, rectSizeY);
    }
}

/**
 * @brief Updates the position and interpolation data of an entity.
 *
 * @param entity Pointer to the entity to update.
 * @param x Target X-coordinate.
 * @param y Target Y-coordinate.
 *
 * @details
 * If the entity has no sprite, sets the position directly.
 * Otherwise, sets target position for interpolation over 0.1 seconds.
 */
void CLIENT::Core::updateEntityPosition(GameEntity* entity, float x, float y)
{
    if (!entity->sprite.has_value() || entity->currentSpritePath.empty()) {
        entity->position = sf::Vector2f(x, y);
        entity->targetPosition = sf::Vector2f(x, y);
        entity->interpolationTime = 0.0f;
        entity->interpolationDuration = 0.1f;
    } else {
        entity->targetPosition = sf::Vector2f(x, y);
        entity->interpolationTime = 0.0f;
        entity->interpolationDuration = 0.1f;
    }

    entity->active = true;
}

/**
 * @brief Updates or assigns a sprite to an entity.
 *
 * @param entity Pointer to the entity.
 * @param entityId Entity ID for logging purposes.
 * @param spritePath Path to the sprite texture.
 * @param needsNewSprite Whether a new sprite must be loaded.
 * @param rectPosX X-coordinate of texture rect.
 * @param rectPosY Y-coordinate of texture rect.
 * @param rectSizeX Width of texture rect.
 * @param rectSizeY Height of texture rect.
 */
void CLIENT::Core::updateEntitySprite(
    GameEntity* entity, uint8_t entityId, const std::string& spritePath,
    bool needsNewSprite, float rectPosX, float rectPosY, float rectSizeX,
    float rectSizeY)
{
    auto& rm = ResourceManager::getInstance();
    sf::Texture* texture = findTexture(spritePath, entityId);

    if (texture && (needsNewSprite || !entity->sprite.has_value())) {
        entity->sprite = sf::Sprite(*texture);
        entity->currentSpritePath = spritePath;
    }

    if (entity->sprite.has_value()) {
        applySpriteTransform(
            entity->sprite.value(), rectPosX, rectPosY, rectSizeX, rectSizeY,
            entity->position);
    }
}

/**
 * @brief Finds a texture from the ResourceManager for a given sprite path.
 *
 * @param spritePath Path to the texture.
 * @param entityId Entity ID for logging.
 * @return Pointer to the SFML texture, or nullptr if not found.
 */
sf::Texture* CLIENT::Core::findTexture(
    const std::string& spritePath, uint8_t entityId)
{
    auto& rm = ResourceManager::getInstance();
    sf::Texture* texture = rm.getTexture(spritePath);

    if (!texture) {
        std::string altPath = spritePath;
        if (altPath.find("assets/") == 0) {
            altPath = altPath.substr(7);
            texture = rm.getTexture(altPath);
        }

        if (!texture) {
            std::cerr << "[Entity " << int(entityId)
                      << "] Texture not found: " << spritePath << "\n";
        }
    }

    return texture;
}

/**
 * @brief Applies a texture rect and transform to an entity's sprite.
 *
 * @param sprite Reference to the SFML sprite.
 * @param rectPosX X-coordinate of the texture rect.
 * @param rectPosY Y-coordinate of the texture rect.
 * @param rectSizeX Width of the texture rect.
 * @param rectSizeY Height of the texture rect.
 * @param position Target position of the sprite.
 */
void CLIENT::Core::applySpriteTransform(
    sf::Sprite& sprite, float rectPosX, float rectPosY, float rectSizeX,
    float rectSizeY, const sf::Vector2f& position)
{
    sprite.setTextureRect(sf::IntRect(
        sf::Vector2i(
            static_cast<int>(std::round(rectPosX)),
            static_cast<int>(std::round(rectPosY))),
        sf::Vector2i(
            static_cast<int>(std::round(rectSizeX)),
            static_cast<int>(std::round(rectSizeY)))));

    sprite.setPosition(position);
    sprite.setScale(sf::Vector2f(2.0f, 2.0f));
}

/**
 * @brief Processes all queued incoming messages from the server.
 *
 * @param window Reference to the game window.
 */
void CLIENT::Core::processIncomingMessages(Window& window)
{
    std::lock_guard<std::mutex> lock(_incomingMutex);

    while (!_incomingMessages.empty()) {
        std::string msg = _incomingMessages.front();
        _incomingMessages.pop();

        handleIncomingMessage(msg, window);
    }
}

/**
 * @brief Handles a single incoming message from the server.
 *
 * @param msg The message string.
 * @param window Reference to the game window.
 */
void CLIENT::Core::handleIncomingMessage(const std::string& msg, Window& window)
{
    if (msg.find("PLAYER_ID:") == 0) {
        _myPlayerId = std::stoi(msg.substr(10));
        std::cout << "*** [Message] Assigned Player ID: " << int(_myPlayerId)
                  << " ***\n";
    } else if (msg.find("PLAYER_JOIN:") == 0) {
        int playerId = std::stoi(msg.substr(12));
        std::cout << "Player " << playerId << " joined\n";
    } else if (msg.find("PLAYER_LEAVE:") == 0) {
        handlePlayerLeave(msg, window);
    } else if (msg.find("TIMEOUT:") == 0) {
        uint8_t playerId = std::stoi(msg.substr(8));
        handleTimeoutEvent(playerId);
        if (playerId == _myPlayerId) {
            window.getWindow().close();
        }
    } else if (msg.find("KILLED:") == 0) {
        uint8_t playerId = std::stoi(msg.substr(7));
        handleKilledEvent(playerId);
    }
}

/**
 * @brief Handles a player leaving the game.
 *
 * @param msg Message string indicating the player leaving.
 * @param window Reference to the game window.
 */
void CLIENT::Core::handlePlayerLeave(const std::string& msg, Window& window)
{
    int playerId = std::stoi(msg.substr(13));
    std::cout << "Player " << playerId << " left\n";

    if (playerId == _myPlayerId) {
        std::cout << "You left or got disconnected\n";
        window.getWindow().close();
    }
}

/**

 * @brief Sends an input action to the server.

 *

 * @param keyCode The key code of the input.

 * @param action The action type (pressed or released).

 *

 * @details

 * Constructs a message in the format "INPUT:<keyCode>:<action>" and

 * pushes it to the outgoing messages queue.

 */

void CLIENT::Core::sendInput(KeyCode keyCode, InputAction action)

{
    std::string inputMsg =

        "INPUT:" + std::to_string(static_cast<uint8_t>(keyCode)) + ":" +

        std::to_string(static_cast<uint8_t>(action));

    std::lock_guard<std::mutex> lock(_outgoingMutex);

    _outgoingMessages.push(inputMsg);
}

/**

 * @brief Handles the change in key state and sends input to the server if
 necessary.

 *

 * @param action The action name (e.g., "MOVE_UP", "SHOOT").

 * @param isPressed True if the key is pressed, false if released.

 * @param keyStates Map storing the current state of each key.

 */

void CLIENT::Core::handleKeyStateChange(

    const std::string& action, bool isPressed,

    std::map<std::string, bool>& keyStates)

{
    if (keyStates[action] == isPressed)

        return;

    keyStates[action] = isPressed;

    for (const auto& mapping : INPUT_MAPPINGS) {
        if (mapping.action == action) {
            InputAction inputAction =

                isPressed ? InputAction::PRESSED : InputAction::RELEASED;

            sendInput(mapping.keyCode, inputAction);

            break;
        }
    }
}

/**

 * @brief Processes pending input actions from the window.

 *

 * @param window Reference to the game window.

 * @param keyStates Map storing the current state of each key.

 *

 * @details

 * Updates key states based on the window's pending actions and sends the

 * corresponding input messages to the server.

 */

void CLIENT::Core::processInputs(

    Window& window, std::map<std::string, bool>& keyStates)

{
    const auto& actions = window.getPendingActions();

    for (const auto& action : actions) {
        handleKeyStateChange(action, true, keyStates);
    }

    for (const auto& mapping : INPUT_MAPPINGS) {
        bool isPressed =

            std::find(actions.begin(), actions.end(), mapping.action) !=

            actions.end();

        if (!isPressed) {
            handleKeyStateChange(mapping.action, false, keyStates);
        }
    }
}

/**

 * @brief Handles a timeout event for a player.

 *

 * @param playerId ID of the player that timed out.

 *

 * @details

 * Updates the game state and stops the client if the local player timed out.

 */

void CLIENT::Core::handleTimeoutEvent(uint8_t playerId)

{
    std::cout << "[CLIENT] Player " << int(playerId) << " timed out\n";

    if (playerId == _myPlayerId) {
        std::cout << "[CLIENT] You have been disconnected due to timeout\n";

        _gameState = GameState::DISCONNECTED;

        _running = false;
    }
}

/**

 * @brief Handles a player being eliminated.

 *

 * @param playerId ID of the eliminated player.

 *

 * @details

 * Updates the game state and loads the defeat screen if the local player

 * was eliminated.

 */

void CLIENT::Core::handleKilledEvent(uint8_t playerId)

{
    std::cout << "[CLIENT] Player " << int(playerId) << " was eliminated\n";

    if (playerId == _myPlayerId) {
        std::cout << "[CLIENT] You have been defeated!\n";

        _gameState = GameState::DEFEATED;

        loadDefeatScreen();
    }
}

/**

 * @brief Loads the defeat screen texture and prepares the sprite.

 *

 * @details

 * Centers the defeat sprite on the window. If loading fails, logs an error.

 */

void CLIENT::Core::loadDefeatScreen()

{
    if (_defeatTextureLoaded)

        return;

    auto& rm = ResourceManager::getInstance();

    rm.loadTexture("assets/sprites/defeat.jpg", "assets/sprites/defeat.jpg");

    sf::Texture* texture = rm.getTexture("assets/sprites/defeat.jpg");

    if (texture) {
        _defeatSprite = sf::Sprite(*texture);

        sf::Vector2u textureSize = texture->getSize();

        _defeatSprite->setOrigin(sf::Vector2f(

            static_cast<float>(textureSize.x) / 2.0f,

            static_cast<float>(textureSize.y) / 2.0f));

        _defeatSprite->setPosition(sf::Vector2f(

            static_cast<float>(WINDOW_WIDTH) / 2.0f,

            static_cast<float>(WINDOW_HEIGHT) / 2.0f));

        _defeatTextureLoaded = true;

        std::cout << "[CLIENT] Defeat screen loaded\n";

    } else {
        std::cerr << "[CLIENT] Failed to load defeat.png\n";
    }
}

/**

 * @brief Renders the defeat screen or a fallback overlay if texture not loaded.

 *

 * @param target The render target to draw on.

 */

void CLIENT::Core::renderDefeatScreen(sf::RenderTarget& target)

{
    if (_defeatTextureLoaded && _defeatSprite.has_value()) {
        target.draw(_defeatSprite.value());

    } else {
        sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));

        overlay.setFillColor(sf::Color(0, 0, 0, 200));

        target.draw(overlay);
    }
}

/**

 * @brief Main graphics loop handling rendering and game updates.

 *

 * @details

 * Handles input processing, snapshot updates, entity updates, parallax,

 * keybind menu, colorblind filter, and rendering of the game and defeat
 screens.

 */
void CLIENT::Core::graphicsLoop()
{
    Window window("R-Type Client", WINDOW_WIDTH, WINDOW_HEIGHT);
    initializeGraphicsComponents();

    std::map<std::string, bool> keyStates = {
        {"MOVE_UP", false},
        {"MOVE_DOWN", false},
        {"MOVE_LEFT", false},
        {"MOVE_RIGHT", false},
        {"SHOOT", false}};

    auto lastCleanup = std::chrono::steady_clock::now();
    const float CLEANUP_INTERVAL = 5.0f;

    _keybindMenu = std::make_unique<KeybindMenu>(*_keybindManager);
    _colorBlindFilter = std::make_unique<ColorBlindFilter>();
    _keybindMenu->setColorBlindFilter(_colorBlindFilter.get());
    window.setKeybindComponents(_keybindManager.get(), _keybindMenu.get());

    sf::RenderTexture renderTexture;
    if (!renderTexture.resize(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT))) {
        std::cerr << "Failed to resize render texture\n";
        return;
    }

    while (window.isOpen() && _running) {
        float deltaTime = window.getDeltaTime();

        updateFromSnapshot();
        processIncomingMessages(window);
        window.pollEvents();

        if (_gameState == GameState::PLAYING) {
            _keybindMenu->update(deltaTime);
            processInputs(window, keyStates);
            _entityManager->update(deltaTime);

            if (shouldCleanupEntities(lastCleanup, CLEANUP_INTERVAL)) {
                _entityManager->cleanupInactiveEntities();
                lastCleanup = std::chrono::steady_clock::now();
            }
        }

        renderTexture.clear();

        if (_gameState == GameState::PLAYING) {
            _entityManager->render(renderTexture);
            _parallaxSystem->update(deltaTime);
            _keybindMenu->render(renderTexture);
        } else if (_gameState == GameState::DEFEATED) {
            _entityManager->render(renderTexture);
            _parallaxSystem->update(deltaTime);
            renderDefeatScreen(renderTexture);
        }

        renderTexture.display();

        window.clear();
        sf::Sprite screenSprite(renderTexture.getTexture());

        if (_colorBlindFilter->isActive()) {
            const sf::RenderStates* states =
                _colorBlindFilter->getRenderStates();
            if (states) {
                window.getWindow().draw(screenSprite, *states);
            } else {
                window.getWindow().draw(screenSprite);
            }
        } else {
            window.getWindow().draw(screenSprite);
        }

        window.display();

        if (_gameState == GameState::DEFEATED) {
            static auto defeatTime = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                               now - defeatTime)
                               .count();

            // if (elapsed >= 5) {
            //     _running = false; if we wanna leave the player watch the game
            // }
        }
    }

    _running = false;
}

/**
 * @brief Initializes graphics components including entity manager and parallax
 * system.
 */
void CLIENT::Core::initializeGraphicsComponents()
{
    auto& rm = ResourceManager::getInstance();

    _entityManager = std::make_unique<EntityManager>();
    _parallaxSystem =
        std::make_unique<ParallaxSystem>(_entityManager.get(), &rm);

    _parallaxSystem->addLayer("assets/sprites/parallax/1.png", 10.0f, 0.1f);
    _parallaxSystem->addLayer("assets/sprites/parallax/2.png", 25.0f, 0.3f);
    _parallaxSystem->addLayer("assets/sprites/parallax/3.png", 50.0f, 0.6f);
    _parallaxSystem->addLayer("assets/sprites/parallax/4.png", 80.0f, 0.9f);

    _parallaxSystem->createLayers();

    std::cout << "Parallax system initialized\n";
}

/**
 * @brief Updates entities from the latest snapshot if available.
 */
void CLIENT::Core::updateFromSnapshot()
{
    std::lock_guard<std::mutex> lock(_snapshotMutex);
    if (_hasNewSnapshot) {
        parseSnapshot(_pendingSnapshot);
        _hasNewSnapshot = false;
    }
}

/**
 * @brief Determines if entity cleanup should occur based on elapsed time.
 *
 * @param lastCleanup Last cleanup timestamp.
 * @param interval Time interval in seconds between cleanups.
 * @return True if cleanup should occur.
 */
bool CLIENT::Core::shouldCleanupEntities(
    const std::chrono::steady_clock::time_point& lastCleanup, float interval)
{
    auto now = std::chrono::steady_clock::now();
    float timeSinceCleanup =
        std::chrono::duration<float>(now - lastCleanup).count();
    return timeSinceCleanup >= interval;
}

/**
 * @brief Renders a frame to the window.
 *
 * @param window Reference to the game window.
 * @param deltaTime Time elapsed since last frame in seconds.
 */
void CLIENT::Core::renderFrame(Window& window, float deltaTime)
{
    window.clear();
    _entityManager->render(window.getWindow());
    _parallaxSystem->update(deltaTime);
    window.display();
}

/**
 * @brief Launches the map editor in a separate window.
 */
void CLIENT::Core::launchMapEditor()
{
    Window window("R-Type Map Editor", WINDOW_WIDTH, WINDOW_HEIGHT);

    auto& rm = ResourceManager::getInstance();
    rm.loadTexture(
        "assets/sprites/r-typesheet42.png", "assets/sprites/r-typesheet42.png");
    rm.loadTexture(
        "assets/sprites/r-typesheet5.png", "assets/sprites/r-typesheet5.png");
    rm.loadTexture(
        "assets/sprites/r-typesheet9.png", "assets/sprites/r-typesheet9.png");
    rm.loadTexture(
        "assets/sprites/r-typesheet10.png", "assets/sprites/r-typesheet10.png");
    rm.loadTexture(
        "assets/sprites/r-typesheet11.png", "assets/sprites/r-typesheet11.png");
    rm.loadTexture(
        "assets/sprites/background.png", "assets/sprites/background.png");
    rm.loadTexture(
        "assets/sprites/playerProjectiles.png",
        "assets/sprites/playerProjectiles.png");
    std::cout << "Map Editor resources loaded\n";

    MapEditor editor;
    editor.toggle();
    std::cout << "Map Editor launched\n";

    while (window.isOpen()) {
        float deltaTime = window.getDeltaTime();

        window.pollEvents();
        editor.handleMouseInput(window.getWindow());
        editor.update(deltaTime);

        window.clear();
        editor.render(window.getWindow());
        editor.renderUI();
        window.display();
    }
}

/**
 * @brief Executes the R-Type client.
 *
 * @param argv Command-line arguments.
 * @return 0 on success, 1 on failure.
 *
 * @details
 * Initializes the Core object and runs the client. Catches and logs exceptions.
 */
int execute_rtypeClient(char** argv)
{
    try {
        CLIENT::Core core(argv);
        core.run();
    } catch (const CLIENT::Core::CoreError& error) {
        std::cerr << "Core error: " << error.what() << std::endl;
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "Unexpected error: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
