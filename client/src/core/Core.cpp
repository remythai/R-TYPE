/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.cpp
*/

#include "Core.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
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

CLIENT::Core::Core(char** argv)
    : _port(0), _running(false), _myPlayerId(255), _hasNewSnapshot(false)
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

CLIENT::Core::~Core()
{
    _running = false;
    if (_networkThread.joinable())
        _networkThread.join();
}

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
}

void CLIENT::Core::handlePlayerIdReceived(uint8_t playerId)
{
    _myPlayerId = playerId;
    std::cout << "*** [Network Callback] Assigned Player ID: "
              << int(_myPlayerId) << " ***\n";

    std::lock_guard<std::mutex> lock(_incomingMutex);
    _incomingMessages.push("PLAYER_ID:" + std::to_string(playerId));
}

void CLIENT::Core::handlePlayerEvent(uint8_t playerId, uint8_t eventType)
{
    std::lock_guard<std::mutex> lock(_incomingMutex);
    if (eventType == 0) {
        _incomingMessages.push("PLAYER_JOIN:" + std::to_string(playerId));
    } else if (eventType == 1) {
        _incomingMessages.push("PLAYER_LEAVE:" + std::to_string(playerId));
    }
}

void CLIENT::Core::handleSnapshotReceived(const std::vector<uint8_t>& payload)
{
    std::lock_guard<std::mutex> lock(_snapshotMutex);
    _pendingSnapshot = payload;
    _hasNewSnapshot = true;
}

void CLIENT::Core::loadResources()
{
    loadGameTextures();
    loadParallaxTextures();
    loadBackgroundMusic();
    std::cout << "Resources loaded\n";
}

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
}

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

void CLIENT::Core::run()
{
    _running = true;
    _networkThread = std::thread(&Core::networkLoop, this);
    graphicsLoop();
    _running = false;
    if (_networkThread.joinable())
        _networkThread.join();
}

void CLIENT::Core::networkLoop()
{
    std::cout << "[Network Thread] Started\n";

    while (_running) {
        processOutgoingMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "[Network Thread] Stopped\n";
}

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

void CLIENT::Core::processIncomingMessages(Window& window)
{
    std::lock_guard<std::mutex> lock(_incomingMutex);

    while (!_incomingMessages.empty()) {
        std::string msg = _incomingMessages.front();
        _incomingMessages.pop();

        handleIncomingMessage(msg, window);
    }
}

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
    }
}

void CLIENT::Core::handlePlayerLeave(const std::string& msg, Window& window)
{
    int playerId = std::stoi(msg.substr(13));
    std::cout << "Player " << playerId << " left\n";

    if (playerId == _myPlayerId) {
        std::cout << "You left or got disconnected\n";
        window.getWindow().close();
    }
}

void CLIENT::Core::sendInput(KeyCode keyCode, InputAction action)
{
    std::string inputMsg =
        "INPUT:" + std::to_string(static_cast<uint8_t>(keyCode)) + ":" +
        std::to_string(static_cast<uint8_t>(action));

    std::lock_guard<std::mutex> lock(_outgoingMutex);
    _outgoingMessages.push(inputMsg);
}

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
    window.setKeybindComponents(_keybindManager.get(), _keybindMenu.get());

    while (window.isOpen() && _running) {
        float deltaTime = window.getDeltaTime();

        updateFromSnapshot();
        processIncomingMessages(window);

        window.pollEvents();
        
        _keybindMenu->update(deltaTime);
        
        processInputs(window, keyStates);
        _entityManager->update(deltaTime);

        if (shouldCleanupEntities(lastCleanup, CLEANUP_INTERVAL)) {
            _entityManager->cleanupInactiveEntities();
            lastCleanup = std::chrono::steady_clock::now();
        }

        window.clear();
        _entityManager->render(window.getWindow());
        _parallaxSystem->update(deltaTime);
        
        _keybindMenu->render(window.getWindow());
        
        window.display();
    }

    _running = false;
}

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

void CLIENT::Core::updateFromSnapshot()
{
    std::lock_guard<std::mutex> lock(_snapshotMutex);
    if (_hasNewSnapshot) {
        parseSnapshot(_pendingSnapshot);
        _hasNewSnapshot = false;
    }
}

bool CLIENT::Core::shouldCleanupEntities(
    const std::chrono::steady_clock::time_point& lastCleanup, float interval)
{
    auto now = std::chrono::steady_clock::now();
    float timeSinceCleanup =
        std::chrono::duration<float>(now - lastCleanup).count();
    return timeSinceCleanup >= interval;
}

void CLIENT::Core::renderFrame(Window& window, float deltaTime)
{
    window.clear();
    _entityManager->render(window.getWindow());
    _parallaxSystem->update(deltaTime);
    window.display();
}

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