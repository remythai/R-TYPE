/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.cpp - Initialization functions
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

struct InputMapping {
    std::string action;
    CLIENT::KeyCode keyCode;
};

static const std::vector<struct InputMapping> INPUT_MAPPINGS = {
    {"MOVE_UP", CLIENT::KeyCode::UP},
    {"MOVE_DOWN", CLIENT::KeyCode::DOWN},
    {"MOVE_LEFT", CLIENT::KeyCode::LEFT},
    {"MOVE_RIGHT", CLIENT::KeyCode::RIGHT},
    {"SHOOT", CLIENT::KeyCode::SHOOT}
};

CLIENT::Core::Core(char **argv)
    : _port(0), _running(false), _myPlayerId(255)
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
        if (payload.empty())
            return;
        
        uint8_t entityCount = payload[0];
        size_t offset = 1;
        const size_t ENTITY_SIZE = 1 + 4 + 4;
        
        for (int i = 0; i < entityCount && (offset + ENTITY_SIZE) <= payload.size(); i++) {
            uint8_t playerId = payload[offset];
            offset += 1;
            
            auto readFloat = [&payload, &offset]() -> float {
                uint32_t temp = (static_cast<uint32_t>(payload[offset]) << 24) | 
                            (static_cast<uint32_t>(payload[offset+1]) << 16) | 
                            (static_cast<uint32_t>(payload[offset+2]) << 8) | 
                            static_cast<uint32_t>(payload[offset+3]);
                offset += 4;
                float result;
                std::memcpy(&result, &temp, sizeof(float));
                return result;
            };
            
            float x = readFloat();
            float y = readFloat();
            offset += 16;
            
            std::lock_guard<std::mutex> lock(_incomingMutex);
            std::string updateMsg = "PLAYER_MOVE:" + std::to_string(playerId) + 
                                ":" + std::to_string(x) + 
                                ":" + std::to_string(y);
            _incomingMessages.push(updateMsg);
            
            if (playerId == _myPlayerId) {
                std::cout << "[Server] My position: (" << x << ", " << y << ")\n";
            }
        }
    });
}

void CLIENT::Core::loadResources()
{
    auto &rm = ResourceManager::getInstance();
    
    rm.loadTexture("player_ships", "sprites/r-typesheet42.png");
    rm.loadTexture("background", "sprites/background.png");
    rm.loadTexture("parallax1.png", "sprites/parallax/1.png");
    rm.loadTexture("parallax2.png", "sprites/parallax/2.png");
    rm.loadTexture("parallax3.png", "sprites/parallax/3.png");
    rm.loadTexture("parallax4.png", "sprites/parallax/4.png");
    rm.loadTexture("projectiles_sheet", "sprites/playerProjectiles.png");
    rm.loadTexture("enemy_type0", "sprites/r-typesheet5.png");
    rm.loadTexture("enemy_type1", "sprites/r-typesheet9.png");
    rm.loadTexture("enemy_type2", "sprites/r-typesheet10.png");
    rm.loadTexture("enemy_type3", "sprites/r-typesheet11.png");

    _backgroundMusic = std::make_unique<sf::Music>();
    if (!_backgroundMusic->openFromFile("sound/backgroundmusic.wav")) {
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

CLIENT::Enemy CLIENT::createTestEnemy(uint32_t id, uint8_t type, float x, float y, 
                             CLIENT::ResourceManager& rm) 
{
    CLIENT::Enemy enemy;
    enemy.entityId = id;
    enemy.enemyType = type;
    enemy.active = true;
    enemy.position = sf::Vector2f(x, y);
    enemy.velocity = sf::Vector2f(-100.0f, 0.0f);
    
    std::string textureName = "enemy_type" + std::to_string(type);
    if (auto* tex = rm.getTexture(textureName)) {
        sf::Vector2u frameSize;
        int frameCount;
        int row = 0;
        float animSpeed;
        
        switch (type) {
            case 0:
                frameSize = sf::Vector2u(33.31, 36);
                frameCount = 8;
                animSpeed = 0.1f;
                break;
            case 1:
                frameSize = sf::Vector2u(55.33, 54);
                frameCount = 3;
                animSpeed = 0.15f;
                break;
            case 2:
                frameSize = sf::Vector2u(33.17, 30);
                frameCount = 6;
                animSpeed = 0.12f;
                break;
            case 3:
                frameSize = sf::Vector2u(33.33, 34);
                frameCount = 3;
                animSpeed = 0.2f;
                break;
            default:
                frameSize = sf::Vector2u(33.33, 34);
                frameCount = 3;
                animSpeed = 0.1f;
        }
        
        enemy.sprite.setAnimation(tex, frameSize, frameCount, row, animSpeed);
        enemy.sprite.play();
        enemy.sprite.setLoop(true);
        enemy.sprite.setScale(2.0f, 2.0f);
        enemy.sprite.setPosition(x, y);
    }
    
    return enemy;
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

void CLIENT::Core::processIncomingMessages(std::array<Player, 4>& players, 
                                          Window& window)
{
    std::lock_guard<std::mutex> lock(_incomingMutex);
    
    while (!_incomingMessages.empty()) {
        std::string msg = _incomingMessages.front();
        _incomingMessages.pop();
        
        if (msg.find("PLAYER_ID:") == 0) {
            _myPlayerId = std::stoi(msg.substr(10));
            players[_myPlayerId].active = true;
            std::cout << "*** Assigned Player ID: " << int(_myPlayerId) << " ***\n";
        }
        else if (msg.find("PLAYER_JOIN:") == 0) {
            int playerId = std::stoi(msg.substr(12));
            if (playerId >= 0 && playerId < 4) {
                players[playerId].active = true;
                std::cout << "Player " << playerId << " joined\n";
            }
        }
        else if (msg.find("PLAYER_LEAVE:") == 0) {
            int playerId = std::stoi(msg.substr(13));
            if (playerId >= 0 && playerId < 4) {
                players[playerId].active = false;
                std::cout << "Player " << playerId << " left\n";
                if (playerId == _myPlayerId) {
                    std::cout << "You left or got disconnected\n";
                    window.getWindow().close();
                }
            }
        }
        else if (msg.find("PLAYER_MOVE:") == 0) {
            size_t pos1 = msg.find(':', 12);
            size_t pos2 = msg.find(':', pos1 + 1);
            
            int playerId = std::stoi(msg.substr(12, pos1 - 12));
            float x = std::stof(msg.substr(pos1 + 1, pos2 - pos1 - 1));
            float y = std::stof(msg.substr(pos2 + 1));
            
            if (playerId >= 0 && playerId < 4) {
                players[playerId].position = sf::Vector2f(x, y);
                players[playerId].sprite.setPosition(x, y);
            }
        }
    }
}

void CLIENT::Core::parseSnapshot(const std::vector<uint8_t>& payload)
{
    if (payload.empty())
        return;
    
    uint8_t entityCount = payload[0];
    size_t offset = 1;
    const size_t ENTITY_SIZE = 25;
    
    for (int i = 0; i < entityCount && (offset + ENTITY_SIZE) <= payload.size(); i++) {
        uint8_t playerId = payload[offset];
        offset += 1;
        
        auto readFloat = [&payload, &offset]() -> float {
            uint32_t temp = (static_cast<uint32_t>(payload[offset]) << 24) | 
                        (static_cast<uint32_t>(payload[offset+1]) << 16) | 
                        (static_cast<uint32_t>(payload[offset+2]) << 8) | 
                        static_cast<uint32_t>(payload[offset+3]);
            offset += 4;
            float result;
            std::memcpy(&result, &temp, sizeof(float));
            return result;
        };
        
        float x = readFloat();
        float y = readFloat();
        offset += 16;
        
        std::lock_guard<std::mutex> lock(_incomingMutex);
        std::string updateMsg = "PLAYER_MOVE:" + std::to_string(playerId) + 
                            ":" + std::to_string(x) + 
                            ":" + std::to_string(y);
        _incomingMessages.push(updateMsg);
        
        if (playerId == _myPlayerId) {
            std::cout << "[Server] My position: (" << x << ", " << y << ")\n";
        }
    }
}

void CLIENT::Core::parseServerEntities(const std::string& message)
{
    std::lock_guard<std::mutex> lock(_incomingMutex);
    
    size_t pos = 0;
    while ((pos = message.find("[P", pos)) != std::string::npos) {
        size_t idStart = pos + 2;
        size_t spacePos = message.find(" ", idStart);
        if (spacePos == std::string::npos) break;
        
        int playerId = std::stoi(message.substr(idStart, spacePos - idStart));
        
        size_t posStart = message.find("pos:(", pos);
        if (posStart == std::string::npos) break;
        posStart += 5;
        
        size_t commaPos = message.find(",", posStart);
        size_t posEnd = message.find(")", commaPos);
        if (commaPos == std::string::npos || posEnd == std::string::npos) break;
        
        float x = std::stof(message.substr(posStart, commaPos - posStart));
        float y = std::stof(message.substr(commaPos + 1, posEnd - commaPos - 1));
        
        std::string updateMsg = "PLAYER_MOVE:" + std::to_string(playerId) + 
                               ":" + std::to_string(x) + 
                               ":" + std::to_string(y);
        _incomingMessages.push(updateMsg);
        
        pos = posEnd + 1;
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
            
            if (action == "SHOOT" && isPressed) {
                std::cout << "Piou piou piou\n";
            }
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

void CLIENT::Core::initializePlayers(std::array<Player, 4>& players)
{
    auto& rm = ResourceManager::getInstance();
    
    if (auto* tex = rm.getTexture("player_ships")) {
        for (int i = 0; i < 4; ++i) {
            players[i].playerId = i;
            players[i].active = false;
            players[i].currentRotation = 2;
            
            players[i].sprite.setAnimation(tex, sf::Vector2u(33.2, 17.2), 5, i, 1.5f);
            players[i].sprite.pause();
            players[i].sprite.setFrame(2);
            
            players[i].position = sf::Vector2f(100.0f, 100.0f + i * 200.0f);
            players[i].sprite.setPosition(players[i].position.x, players[i].position.y);
            players[i].sprite.setScale(2.0f, 2.0f);
        }
    }
}

void CLIENT::Core::spawnTestEnemies(std::unordered_map<uint32_t, Enemy>& enemies, 
                                   uint32_t& nextEnemyId)
{
    auto& rm = ResourceManager::getInstance();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> typeDist(0, 3);
    std::uniform_real_distribution<> yDist(50, WINDOW_HEIGHT - 100);
    
    for (int i = 0; i < 5; ++i) {
        uint8_t type = typeDist(gen);
        float y = yDist(gen);
        float x = WINDOW_WIDTH + 100 + (i * 150);
        
        Enemy enemy = createTestEnemy(nextEnemyId++, type, x, y, rm);
        enemies[enemy.entityId] = std::move(enemy);
        
        std::cout << "[TEST] Created enemy type " << int(type) 
                  << " at (" << x << ", " << y << ")\n";
    }
}

void CLIENT::Core::updateEnemySpawning(std::unordered_map<uint32_t, Enemy>& enemies,
                                      uint32_t& nextEnemyId, float& spawnTimer, 
                                      float deltaTime)
{
    const float ENEMY_SPAWN_INTERVAL = 2.0f;
    spawnTimer += deltaTime;
    
    if (spawnTimer >= ENEMY_SPAWN_INTERVAL) {
        spawnTimer = 0.0f;
        
        auto& rm = ResourceManager::getInstance();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> typeDist(0, 3);
        std::uniform_real_distribution<> yDist(50, WINDOW_HEIGHT - 100);
        
        uint8_t type = typeDist(gen);
        float y = yDist(gen);
        float x = WINDOW_WIDTH + 100;
        
        Enemy enemy = createTestEnemy(nextEnemyId++, type, x, y, rm);
        enemies[enemy.entityId] = std::move(enemy);
        
        std::cout << "[TEST] Spawned enemy type " << int(type) 
                  << " at (" << x << ", " << y << ")\n";
    }
}

void CLIENT::Core::updateEnemies(std::unordered_map<uint32_t, Enemy>& enemies, 
                                float deltaTime)
{
    for (auto& [id, enemy] : enemies) {
        enemy.position += enemy.velocity * deltaTime;
        enemy.sprite.setPosition(enemy.position.x, enemy.position.y);
        enemy.sprite.update(deltaTime);
    }
}

void CLIENT::Core::cleanupEnemies(std::unordered_map<uint32_t, Enemy>& enemies)
{
    auto it = enemies.begin();
    while (it != enemies.end()) {
        if (it->second.position.x < -100) {
            std::cout << "[TEST] Enemy " << it->first << " removed (off screen)\n";
            it = enemies.erase(it);
        } else {
            ++it;
        }
    }
}

void CLIENT::Core::updateProjectiles(std::vector<LocalProjectile>& projectiles, 
                                    float deltaTime)
{
    for (auto& projectile : projectiles) {
        projectile.position += projectile.velocity * deltaTime;
        projectile.sprite.setPosition(projectile.position.x, projectile.position.y);
        projectile.sprite.update(deltaTime);
    }
}

void CLIENT::Core::cleanupProjectiles(std::vector<LocalProjectile>& projectiles)
{
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const LocalProjectile& p) {
                return p.position.x > WINDOW_WIDTH + 50 || p.position.x < -50 ||
                       p.position.y > WINDOW_HEIGHT + 50 || p.position.y < -50;
            }),
        projectiles.end()
    );
}

void CLIENT::Core::renderScene(Window& window, const std::array<Player, 4>& players,
                              const std::unordered_map<uint32_t, Enemy>& enemies,
                              const std::vector<LocalProjectile>& projectiles)
{
    window.clear();
    _entityManager->render(window.getWindow());

    for (const auto& player : players) {
        if (player.active) {
            player.sprite.draw(window.getWindow());
        }
    }

    for (const auto& [id, enemy] : enemies) {
        if (enemy.active) {
            enemy.sprite.draw(window.getWindow());
        }
    }

    for (const auto& projectile : projectiles) {
        projectile.sprite.draw(window.getWindow());
    }

    window.display();
}

void CLIENT::Core::graphicsLoop()
{
    Window window("R-Type Client", WINDOW_WIDTH, WINDOW_HEIGHT);
    auto& rm = ResourceManager::getInstance();

    _entityManager = std::make_unique<EntityManager>();
    _parallaxSystem = std::make_unique<ParallaxSystem>(_entityManager.get(), &rm);
    _parallaxSystem->createParallaxLayers();
    std::cout << "Parallax system initialized with 4 layers\n";

    std::array<Player, 4> players;
    initializePlayers(players);
    
    std::unordered_map<uint32_t, Enemy> enemies;
    std::vector<LocalProjectile> localProjectiles;
    
    std::map<std::string, bool> keyStates = {
        {"MOVE_UP", false}, {"MOVE_DOWN", false}, {"MOVE_LEFT", false},
        {"MOVE_RIGHT", false}, {"SHOOT", false}
    };

    uint32_t nextEnemyId = 1000;
    float enemySpawnTimer = 0.0f;
    spawnTestEnemies(enemies, nextEnemyId);
    
    _myPlayerId = 0;
    players[0].active = true;

    while (window.isOpen() && _running) {
        float deltaTime = window.getDeltaTime();
        
        processIncomingMessages(players, window);
        
        window.pollEvents();
        processInputs(window, keyStates);
        
        updateEnemySpawning(enemies, nextEnemyId, enemySpawnTimer, deltaTime);
        
        updateEnemies(enemies, deltaTime);
        cleanupEnemies(enemies);
        
        updateProjectiles(localProjectiles, deltaTime);
        cleanupProjectiles(localProjectiles);
        
        _entityManager->update(deltaTime);
        
        renderScene(window, players, enemies, localProjectiles);
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