/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.cpp
*/

#include "Core.hpp"
#include "../graphics/Window.hpp"
#include "../graphics/ResourceManager.hpp"
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <array>
#include <algorithm>
#include <random>
#include <sstream>

struct InputMapping {
    std::string action;
    CLIENT::KeyCode keyCode;
};

static const std::vector<struct InputMapping> INPUT_MAPPINGS = {
    {"MOVE_DOWN", CLIENT::KeyCode::DOWN},
    {"MOVE_UP", CLIENT::KeyCode::UP},
    {"MOVE_LEFT", CLIENT::KeyCode::LEFT},
    {"MOVE_RIGHT", CLIENT::KeyCode::RIGHT},
    {"SHOOT", CLIENT::KeyCode::SHOOT}
};

CLIENT::Core::Core(char **argv)
    : _port(0), _running(false), _myPlayerId(255), _hasNewSnapshot(false)
{
    parseCommandLineArgs(argv);
    
    std::cout << "Enter username: ";
    std::getline(std::cin, _username);
    if (_username.empty())
        _username = "Player";

    initializeNetwork();
    loadResources();
}

CLIENT::Core::~Core()
{
    _running = false;
    if (_networkThread.joinable())
        _networkThread.join();
}

void CLIENT::Core::parseCommandLineArgs(char **argv)
{
    for (int i = 1; argv[i]; ++i) {
        std::string arg = argv[i];
        if (arg == "-p") {
            if (!argv[i + 1]) throw CoreError("Missing value for -p");
            _port = std::stoi(argv[++i]);
        } else if (arg == "-h") {
            if (!argv[i + 1]) throw CoreError("Missing value for -h");
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

    std::cout << "[Core] Client initialized with username: " << _username << "\n";
    std::cout << "[Core] Waiting for player ID from server...\n";
}

void CLIENT::Core::setupNetworkCallbacks()
{
    _networkClient->setOnPlayerIdReceived([this](uint8_t playerId) {
        _myPlayerId = playerId;
        std::cout << "*** [Network Callback] Assigned Player ID: " << int(_myPlayerId) << " ***\n";
        
        std::lock_guard<std::mutex> lock(_incomingMutex);
        _incomingMessages.push("PLAYER_ID:" + std::to_string(playerId));
    });
    
    _networkClient->setOnPlayerEvent([this](uint8_t playerId, uint8_t eventType) {
        std::lock_guard<std::mutex> lock(_incomingMutex);
        if (eventType == 0) {
            _incomingMessages.push("PLAYER_JOIN:" + std::to_string(playerId));
        } else if (eventType == 1) {
            _incomingMessages.push("PLAYER_LEAVE:" + std::to_string(playerId));
        }
    });
    
    _networkClient->setOnSnapshot([this](const std::vector<uint8_t>& payload) {
        std::lock_guard<std::mutex> lock(_snapshotMutex);
        _pendingSnapshot = payload;
        _hasNewSnapshot = true;
    });
}

void CLIENT::Core::loadResources()
{
    auto &rm = ResourceManager::getInstance();
    
    rm.loadTexture("assets/sprites/r-typesheet42.png", "assets/sprites/r-typesheet42.png");
    rm.loadTexture("assets/sprites/r-typesheet42.png", "sprites/r-typesheet42.png");
    rm.loadTexture("assets/sprites/playerProjectiles.png", "assets/sprites/playerProjectiles.png");
    rm.loadTexture("assets/sprites/r-typesheet5.png", "assets/sprites/r-typesheet5.png");
    rm.loadTexture("assets/sprites/r-typesheet9.png", "assets/sprites/r-typesheet9.png");
    rm.loadTexture("assets/sprites/r-typesheet10.png", "assets/sprites/r-typesheet10.png");
    rm.loadTexture("assets/sprites/r-typesheet11.png", "assets/sprites/r-typesheet11.png");
    
    rm.loadTexture("assets/sprites/background.png", "assets/sprites/background.png");
    rm.loadTexture("parallax1.png", "assets/sprites/parallax/1.png");
    rm.loadTexture("parallax2.png", "assets/sprites/parallax/2.png");
    rm.loadTexture("parallax3.png", "assets/sprites/parallax/3.png");
    rm.loadTexture("parallax4.png", "assets/sprites/parallax/4.png");
    rm.loadTexture("assets/sprites/parallax/1.png", "assets/sprites/parallax/1.png");
    rm.loadTexture("assets/sprites/parallax/2.png", "assets/sprites/parallax/2.png");
    rm.loadTexture("assets/sprites/parallax/3.png", "assets/sprites/parallax/3.png");
    rm.loadTexture("assets/sprites/parallax/4.png", "assets/sprites/parallax/4.png");

    _backgroundMusic = std::make_unique<sf::Music>();
    if (!_backgroundMusic->openFromFile("assets/sound/backgroundmusic.wav")) {
        std::cerr << "Failed to load background music\n";
    } else {
        _backgroundMusic->setLooping(true);
        _backgroundMusic->setVolume(100);
        _backgroundMusic->play();
    }

    std::cout << "Resources loaded\n";
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
            if (_myPlayerId == 255) {
                std::cout << "[Network] Cannot send input, no player ID assigned yet\n";
                continue;
            }
            
            size_t pos1 = msg.find(':', 6);
            uint8_t keyCode = std::stoi(msg.substr(6, pos1 - 6));
            uint8_t action = std::stoi(msg.substr(pos1 + 1));
            
            _networkClient->sendInput(_myPlayerId, keyCode, action);
        }
    }
}

void CLIENT::Core::parseSnapshot(const std::vector<uint8_t>& payload)
{
    if (payload.empty()) return;
    
    size_t offset = 0;
    auto& rm = ResourceManager::getInstance();
    
    std::set<uint8_t> activeEntitiesInSnapshot;
    
    auto readFloat = [&payload, &offset]() -> float {
        if (offset + 4 > payload.size()) return 0.0f;
        uint32_t temp = (static_cast<uint32_t>(payload[offset]) << 24) | 
                        (static_cast<uint32_t>(payload[offset+1]) << 16) | 
                        (static_cast<uint32_t>(payload[offset+2]) << 8) | 
                        static_cast<uint32_t>(payload[offset+3]);
        offset += 4;
        float result;
        std::memcpy(&result, &temp, sizeof(float));
        return result;
    };
    
    while (offset < payload.size()) {
        if (offset >= payload.size()) break;
        uint8_t entityId = payload[offset++];
        
        if (offset + 8 > payload.size()) break;
        float x = readFloat();
        float y = readFloat();
        
        if (offset >= payload.size()) break;
        uint8_t pathLen = payload[offset++];
        
        if (offset + pathLen > payload.size()) break;
        std::string spritePath(payload.begin() + offset, payload.begin() + offset + pathLen);
        offset += pathLen;
        
        if (offset + 2 > payload.size()) break;
        uint8_t currentFrame = payload[offset++];
        uint8_t frameCount = payload[offset++];
        
        if (offset + 4 > payload.size()) break;
        float frameDuration = readFloat();
        
        if (offset + 8 > payload.size()) break;
        float rectPosX = readFloat();
        float rectPosY = readFloat();
        
        if (offset + 8 > payload.size()) break;
        float rectSizeX = readFloat();
        float rectSizeY = readFloat();
        
        activeEntitiesInSnapshot.insert(entityId);
        
        GameEntity* entity = _entityManager->getEntity(entityId);
        bool needsNewSprite = false;
        
        if (entity) {
            if (!entity->currentSpritePath.empty() && 
                entity->currentSpritePath != spritePath) {
                
                std::cout << "[Entity " << int(entityId) 
                          << "] ⚠️  Sprite path changed:\n"
                          << "    OLD: '" << entity->currentSpritePath << "'\n"
                          << "    NEW: '" << spritePath << "'\n"
                          << "    → Recreating sprite\n";
                
                needsNewSprite = true;
                
                EntityType newType = determineEntityType(entityId, spritePath);
                RenderLayer newLayer = determineRenderLayer(newType);
                
                if (entity->type != newType || entity->layer != newLayer) {
                    std::cout << "[Entity " << int(entityId) 
                              << "] Type/Layer changed: "
                              << int(entity->type) << "/" << int(entity->layer)
                              << " → " << int(newType) << "/" << int(newLayer) << "\n";
                    
                    entity->type = newType;
                    entity->layer = newLayer;
                }
            }
        } else {
            EntityType type = determineEntityType(entityId, spritePath);
            RenderLayer layer = determineRenderLayer(type);
            
            _entityManager->createServerEntity(entityId, type, layer);
            entity = _entityManager->getEntity(entityId);
            needsNewSprite = true;
            
            std::cout << "[Entity " << int(entityId) 
                      << "] ✓ Created new entity (Type: " << int(type) 
                      << ", Layer: " << int(layer) << ")\n";
        }
        
        if (entity) {
            entity->position = sf::Vector2f(x, y);
            entity->active = true;
            
            sf::Texture* texture = rm.getTexture(spritePath);
            if (!texture) {
                std::string altPath = spritePath;
                if (altPath.find("assets/") == 0) {
                    altPath = altPath.substr(7);
                    texture = rm.getTexture(altPath);
                }
                
                if (!texture) {
                    std::cerr << "[Entity " << int(entityId) 
                              << "] ❌ Texture not found: " << spritePath << "\n";
                }
            }
            
            if (texture && (needsNewSprite || !entity->sprite.has_value())) {
                entity->sprite = sf::Sprite(*texture);
                entity->currentSpritePath = spritePath;
                
                std::cout << "[Entity " << int(entityId) 
                          << "] ✓ Sprite " << (needsNewSprite ? "recreated" : "created") 
                          << " with: " << spritePath << "\n";
            }
            
            if (entity->sprite.has_value()) {
                entity->sprite->setTextureRect(sf::IntRect(
                    sf::Vector2i(static_cast<int>(rectPosX + currentFrame * rectSizeX), 
                                 static_cast<int>(rectPosY)),
                    sf::Vector2i(static_cast<int>(rectSizeX), 
                                 static_cast<int>(rectSizeY))
                ));
                
                entity->sprite->setPosition(sf::Vector2f(x, y));
                entity->sprite->setScale(sf::Vector2f(2.0f, 2.0f));
            }
        }
    }
    
    _entityManager->deactivateEntitiesNotInSet(activeEntitiesInSnapshot);
}

void CLIENT::Core::processIncomingMessages(Window& window)
{
    std::lock_guard<std::mutex> lock(_incomingMutex);
    
    while (!_incomingMessages.empty()) {
        std::string msg = _incomingMessages.front();
        _incomingMessages.pop();
        
        if (msg.find("PLAYER_ID:") == 0) {
            _myPlayerId = std::stoi(msg.substr(10));
            std::cout << "*** [Message] Assigned Player ID: " << int(_myPlayerId) << " ***\n";
        }
        else if (msg.find("PLAYER_JOIN:") == 0) {
            int playerId = std::stoi(msg.substr(12));
            std::cout << "Player " << playerId << " joined\n";
        }
        else if (msg.find("PLAYER_LEAVE:") == 0) {
            int playerId = std::stoi(msg.substr(13));
            std::cout << "Player " << playerId << " left\n";
            if (playerId == _myPlayerId) {
                std::cout << "You left or got disconnected\n";
                window.getWindow().close();
            }
        }
    }
}

CLIENT::EntityType CLIENT::Core::determineEntityType(uint32_t entityId, 
                                                      const std::string& spritePath)
{
    if (spritePath.find("r-typesheet42") != std::string::npos) {
        return EntityType::PLAYER;
    } else if (spritePath.find("Projectile") != std::string::npos) {
        return EntityType::PROJECTILE;
    } else if (spritePath.find("r-typesheet5") != std::string::npos ||
               spritePath.find("r-typesheet9") != std::string::npos ||
               spritePath.find("r-typesheet10") != std::string::npos ||
               spritePath.find("r-typesheet11") != std::string::npos) {
        return EntityType::ENEMY;
    }
    
    if (entityId < 100) return EntityType::PLAYER;
    if (entityId < 1000) return EntityType::PROJECTILE;
    return EntityType::ENEMY;
}

CLIENT::RenderLayer CLIENT::Core::determineRenderLayer(EntityType type)
{
    switch (type) {
        case EntityType::PLAYER:
            return RenderLayer::PLAYERS;
        case EntityType::ENEMY:
            return RenderLayer::ENEMIES;
        case EntityType::PROJECTILE:
            return RenderLayer::PROJECTILES;
        case EntityType::OBSTACLE:
            return RenderLayer::OBSTACLES;
        case EntityType::BACKGROUND:
            return RenderLayer::BACKGROUND;
        default:
            return RenderLayer::ENEMIES;
    }
}

void CLIENT::Core::sendInput(KeyCode keyCode, InputAction action)
{
    std::string inputMsg = "INPUT:" + std::to_string(static_cast<uint8_t>(keyCode)) + ":" 
                         + std::to_string(static_cast<uint8_t>(action));
    
    std::lock_guard<std::mutex> lock(_outgoingMutex);
    _outgoingMessages.push(inputMsg);
}

void CLIENT::Core::handleKeyStateChange(const std::string& action, bool isPressed, 
                                       std::map<std::string, bool>& keyStates)
{
    if (keyStates[action] == isPressed)
        return;
    
    keyStates[action] = isPressed;
    
    for (const auto& mapping : INPUT_MAPPINGS) {
        if (mapping.action == action) {
            InputAction inputAction = isPressed ? InputAction::PRESSED : InputAction::RELEASED;
            sendInput(mapping.keyCode, inputAction);
            
            break;
        }
    }
}

void CLIENT::Core::processInputs(Window& window, std::map<std::string, bool>& keyStates)
{
    const auto& actions = window.getPendingActions();
    
    for (const auto& action : actions) {
        handleKeyStateChange(action, true, keyStates);
    }
    
    for (const auto& mapping : INPUT_MAPPINGS) {
        bool isPressed = std::find(actions.begin(), actions.end(), mapping.action) != actions.end();
        if (!isPressed) {
            handleKeyStateChange(mapping.action, false, keyStates);
        }
    }
}

void CLIENT::Core::graphicsLoop()
{
    Window window("R-Type Client", WINDOW_WIDTH, WINDOW_HEIGHT);
    auto& rm = ResourceManager::getInstance();

    _entityManager = std::make_unique<EntityManager>();
    _parallaxSystem = std::make_unique<ParallaxSystem>(_entityManager.get(), &rm);
    _parallaxSystem->createParallaxLayers();
    std::cout << "Parallax system initialized with 4 layers\n";
    
    std::map<std::string, bool> keyStates = {
        {"MOVE_UP", false}, {"MOVE_DOWN", false}, {"MOVE_LEFT", false},
        {"MOVE_RIGHT", false}, {"SHOOT", false}
    };

    auto lastCleanup = std::chrono::steady_clock::now();
    const float CLEANUP_INTERVAL = 5.0f;


    while (window.isOpen() && _running) {
        float deltaTime = window.getDeltaTime();
        
        {
            std::lock_guard<std::mutex> lock(_snapshotMutex);
            if (_hasNewSnapshot) {
                parseSnapshot(_pendingSnapshot);
                _hasNewSnapshot = false;
            }
        }
        
        processIncomingMessages(window);
        
        window.pollEvents();
        processInputs(window, keyStates);
        
        _entityManager->update(deltaTime);
        
        auto now = std::chrono::steady_clock::now();
        float timeSinceCleanup = std::chrono::duration<float>(now - lastCleanup).count();
        if (timeSinceCleanup >= CLEANUP_INTERVAL) {
            _entityManager->cleanupInactiveEntities();
            lastCleanup = now;
        }
        
        window.clear();
        _entityManager->render(window.getWindow());
        window.display();
    }

    _running = false;
}

int execute_rtypeClient(char **argv)
{
    try {
        CLIENT::Core core(argv);
        core.run();
    } catch (const CLIENT::Core::CoreError &error) {
        std::cerr << "Core error: " << error.what() << std::endl;
        return 1;
    } catch (const std::exception &error) {
        std::cerr << "Unexpected error: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}