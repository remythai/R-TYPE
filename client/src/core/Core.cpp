/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.cpp
*/

#include "Core.hpp"
#include "../graphics/Window.hpp"
#include "../graphics/ResourceManager.hpp"
#include "../graphics/AnimatedSprite.hpp"
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <array>
#include <algorithm>

struct InputMapping {
    std::string action;
    CLIENT::KeyCode keyCode;
};

static const std::vector<InputMapping> INPUT_MAPPINGS = {
    {"MOVE_UP", CLIENT::KeyCode::UP},
    {"MOVE_DOWN", CLIENT::KeyCode::DOWN},
    {"MOVE_LEFT", CLIENT::KeyCode::LEFT},
    {"MOVE_RIGHT", CLIENT::KeyCode::RIGHT},
    {"SHOOT", CLIENT::KeyCode::SHOOT}
};

struct LocalProjectile {
    uint32_t id;
    sf::Vector2f position;
    sf::Vector2f velocity;
    CLIENT::AnimatedSprite sprite;
};

CLIENT::Core::Core(char **argv)
    : _port(0), _running(false), _myPlayerId(255)
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

    std::cout << "Enter username: ";
    std::getline(std::cin, _username);
    if (_username.empty())
        _username = "Player";

    _networkClient = std::make_unique<NetworkClient>(_hostname, _port);
    
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
    
    _networkClient->sendJoin(_username);
    _networkClient->startReceiving();

    std::cout << "[Core] Client initialized with username: " << _username << "\n";
    std::cout << "[Core] Waiting for player ID from server...\n";
    
    loadResources();
}

CLIENT::Core::~Core()
{
    _running = false;
    if (_networkThread.joinable())
        _networkThread.join();
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

void CLIENT::Core::networkLoop()
{
    std::cout << "[Network Thread] Started\n";

    while (_running) {
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
                    std::cout << "[Network] Sent INPUT: playerId=" << int(_myPlayerId)
                              << ", keyCode=" << int(keyCode) 
                              << ", action=" << int(action) << "\n";
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "[Network Thread] Stopped\n";
}

void CLIENT::Core::sendInput(CLIENT::KeyCode keyCode, CLIENT::InputAction action)
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
            CLIENT::InputAction inputAction = isPressed ? CLIENT::InputAction::PRESSED : CLIENT::InputAction::RELEASED;
            sendInput(mapping.keyCode, inputAction);
            
            if (action == "SHOOT" && isPressed) {
                std::cout << "Piou piou piou\n";
            }
            break;
        }
    }
}

void CLIENT::Core::graphicsLoop()
{
    CLIENT::Window window("R-Type Client", WINDOW_WIDTH, WINDOW_HEIGHT);
    auto& rm = ResourceManager::getInstance();

    _entityManager = std::make_unique<EntityManager>();
    _parallaxSystem = std::make_unique<ParallaxSystem>(_entityManager.get(), &rm);
    
    _parallaxSystem->createParallaxLayers(); 

    std::cout << "Parallax system initialized with 4 layers\n";


    std::array<Player, 4> players;

    if (auto* tex = rm.getTexture("player_ships")) {
        for (int i = 0; i < 4; ++i) {
            players[i].playerId = i;
            players[i].active = false;
            players[i].currentRotation = 2;
            
            players[i].sprite.setAnimation(
                tex,
                sf::Vector2u(33.2, 17.2),
                5,
                i,
                1.5f 
            );
            players[i].sprite.pause();
            players[i].sprite.setFrame(2);
            
            players[i].position = sf::Vector2f(100.0f, 100.0f + i * 200.0f);
            players[i].sprite.setPosition(players[i].position.x, players[i].position.y);
            players[i].sprite.setScale(2.0f, 2.0f);
        }
    }
    
    std::map<std::string, bool> keyStates = {
        {"MOVE_UP", false},
        {"MOVE_DOWN", false},
        {"MOVE_LEFT", false},
        {"MOVE_RIGHT", false},
        {"SHOOT", false}
    };

    std::unordered_map<uint32_t, CLIENT::AnimatedSprite> projectileSprites;
    
    std::vector<LocalProjectile> localProjectiles;
    uint32_t nextProjectileId = 10000;
    
    float shootCooldown = 0.0f;
    const float SHOOT_DELAY = 0.2f;

    bool waitingForPlayerId = true;

    while (window.isOpen() && _running) {
        float deltaTime = window.getDeltaTime();
        
        if (shootCooldown > 0.0f) {
            shootCooldown -= deltaTime;
        }
        
        {
            std::lock_guard<std::mutex> lock(_incomingMutex);
            while (!_incomingMessages.empty()) {
                std::string msg = _incomingMessages.front();
                _incomingMessages.pop();
                
                std::cout << "[Graphics] Received from network: " << msg << "\n";
                
                if (msg.find("PLAYER_ID:") == 0) {
                    _myPlayerId = std::stoi(msg.substr(10));
                    players[_myPlayerId].active = true;
                    waitingForPlayerId = false;
                    std::cout << "*** Assigned Player ID: " << int(_myPlayerId) << " ***\n";
                    std::cout << "*** Using ship " << int(_myPlayerId) + 1 << "/4 ***\n";
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
                    }
                }
                else if (msg.find("PLAYER_MOVE:") == 0) {
                    size_t pos1 = msg.find(':', 12);
                    size_t pos2 = msg.find(':', pos1 + 1);
                    
                    int playerId = std::stoi(msg.substr(12, pos1 - 12));
                    float x = std::stof(msg.substr(pos1 + 1, pos2 - pos1 - 1));
                    float y = std::stof(msg.substr(pos2 + 1));
                    
                    if (playerId >= 0 && playerId < 4 && playerId != _myPlayerId) {
                        players[playerId].position = sf::Vector2f(x, y);
                        players[playerId].sprite.setPosition(x, y);
                    }
                }
                else if (msg.find("ENEMY_SPAWN:") == 0) {
                    size_t pos1 = msg.find(':', 12);
                    size_t pos2 = msg.find(':', pos1 + 1);
                    size_t pos3 = msg.find(':', pos2 + 1);
                    
                    uint32_t entityId = std::stoi(msg.substr(12, pos1 - 12));
                    float x = std::stof(msg.substr(pos1 + 1, pos2 - pos1 - 1));
                    float y = std::stof(msg.substr(pos2 + 1, pos3 - pos2 - 1));
                    uint8_t enemyType = std::stoi(msg.substr(pos3 + 1));
                    
                    _entityManager->createServerEntity(entityId, EntityType::ENEMY, RenderLayer::ENEMIES);
                    auto* enemy = _entityManager->getEntity(entityId);
                    
                    if (enemy) {
                        std::string textureName = "enemy_type" + std::to_string(enemyType);
                        if (auto* tex = rm.getTexture(textureName)) {
                            enemy->sprite = sf::Sprite(*tex);
                            enemy->sprite->setScale(sf::Vector2f(2.0f, 2.0f));
                        }
                        
                        enemy->position = sf::Vector2f(x, y);
                        if (enemy->sprite.has_value())
                            enemy->sprite->setPosition(enemy->position);
                        
                        std::cout << "Enemy " << entityId << " spawned at (" << x << ", " << y << ")\n";
                    }
                }
                else if (msg.find("ENEMY_MOVE:") == 0) {
                    size_t pos1 = msg.find(':', 11);
                    size_t pos2 = msg.find(':', pos1 + 1);
                    
                    uint32_t entityId = std::stoi(msg.substr(11, pos1 - 11));
                    float x = std::stof(msg.substr(pos1 + 1, pos2 - pos1 - 1));
                    float y = std::stof(msg.substr(pos2 + 1));
                    
                    auto* enemy = _entityManager->getEntity(entityId);
                    if (enemy && enemy->sprite.has_value()) {
                        enemy->position = sf::Vector2f(x, y);
                        enemy->sprite->setPosition(enemy->position);
                    }
                }
                else if (msg.find("ENEMY_DESTROY:") == 0) {
                    uint32_t entityId = std::stoi(msg.substr(14));
                    _entityManager->removeEntity(entityId);
                    std::cout << "Enemy " << entityId << " destroyed\n";
                }
                else if (msg.find("OBSTACLE_SPAWN:") == 0) {
                    size_t pos1 = msg.find(':', 15);
                    size_t pos2 = msg.find(':', pos1 + 1);
                    size_t pos3 = msg.find(':', pos2 + 1);
                    
                    uint32_t entityId = std::stoi(msg.substr(15, pos1 - 15));
                    float x = std::stof(msg.substr(pos1 + 1, pos2 - pos1 - 1));
                    float y = std::stof(msg.substr(pos2 + 1, pos3 - pos2 - 1));
                    uint8_t obstacleType = std::stoi(msg.substr(pos3 + 1));
                    
                    std::string textureName = "asteroid" + std::to_string(obstacleType % 2 + 1);
                    _parallaxSystem->createObstacle(entityId, x, y, textureName, obstacleType);
                    
                    std::cout << "Obstacle " << entityId << " spawned at (" << x << ", " << y << ")\n";
                }
                else if (msg.find("OBSTACLE_MOVE:") == 0) {
                    size_t pos1 = msg.find(':', 14);
                    size_t pos2 = msg.find(':', pos1 + 1);
                    
                    uint32_t entityId = std::stoi(msg.substr(14, pos1 - 14));
                    float x = std::stof(msg.substr(pos1 + 1, pos2 - pos1 - 1));
                    float y = std::stof(msg.substr(pos2 + 1));
                    
                    auto* obstacle = _entityManager->getEntity(entityId);
                    if (obstacle && obstacle->sprite.has_value()) {
                        obstacle->position = sf::Vector2f(x, y);
                        obstacle->sprite->setPosition(obstacle->position);
                    }
                }
                else if (msg.find("OBSTACLE_DESTROY:") == 0) {
                    uint32_t entityId = std::stoi(msg.substr(17));
                    _entityManager->removeEntity(entityId);
                    std::cout << "Obstacle " << entityId << " destroyed\n";
                }
                else if (msg.find("PROJECTILE_SPAWN:") == 0) {
                    size_t pos1 = msg.find(':', 17);
                    size_t pos2 = msg.find(':', pos1 + 1);
                    size_t pos3 = msg.find(':', pos2 + 1);
                    
                    uint32_t entityId = std::stoi(msg.substr(17, pos1 - 17));
                    float x = std::stof(msg.substr(pos1 + 1, pos2 - pos1 - 1));
                    float y = std::stof(msg.substr(pos2 + 1, pos3 - pos2 - 1));
                    uint8_t ownerId = std::stoi(msg.substr(pos3 + 1));
                    
                    std::cout << ">>> PROJECTILE_SPAWN: ID=" << entityId << " at (" << x << ", " << y << ")\n";
                    
                    if (auto* tex = rm.getTexture("projectiles_sheet")) {
                        CLIENT::AnimatedSprite animSprite;
                        
                        animSprite.setAnimation(
                            tex,
                sf::Vector2u(22.8, 22.8),
                3,
                      0,
            0.05f
                        );
                        
                        animSprite.play();
                        animSprite.setLoop(true);
                        animSprite.setScale(2.0f, 2.0f);
                        animSprite.setPosition(x, y);
                        
                        projectileSprites[entityId] = std::move(animSprite);
                        
                        std::cout << "    Projectile sprite created (total: " 
                                << projectileSprites.size() << ")\n";
                    } else {
                        std::cerr << "    ERROR: projectiles_sheet texture not found!\n";
                    }
                }
                else if (msg.find("PROJECTILE_MOVE:") == 0) {
                    size_t pos1 = msg.find(':', 16);
                    size_t pos2 = msg.find(':', pos1 + 1);
                    
                    uint32_t entityId = std::stoi(msg.substr(16, pos1 - 16));
                    float x = std::stof(msg.substr(pos1 + 1, pos2 - pos1 - 1));
                    float y = std::stof(msg.substr(pos2 + 1));
                    
                    auto it = projectileSprites.find(entityId);
                    if (it != projectileSprites.end()) {
                        it->second.setPosition(x, y);
                    }
                }
                else if (msg.find("PROJECTILE_DESTROY:") == 0) {
                    uint32_t entityId = std::stoi(msg.substr(19));
                    projectileSprites.erase(entityId);
                    std::cout << "Projectile " << entityId << " destroyed (remaining: " 
                              << projectileSprites.size() << ")\n";
                }
            }
        }

        window.pollEvents();
        
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
        
        if (_myPlayerId != 255) {
            auto &player = players[_myPlayerId];
            float speed = 200.0f * deltaTime;

            sf::Vector2f spriteSize(0.f, 0.f);
            sf::Sprite* baseSprite = player.sprite.getSprite();

            if (baseSprite) {
                sf::IntRect rect = baseSprite->getTextureRect();
                spriteSize.x = static_cast<float>(rect.size.x);
                spriteSize.y = static_cast<float>(rect.size.y);
            }

            bool movingUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
            bool movingDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);

            if (movingUp) {
                if (player.currentRotation < 4) {
                    player.currentRotation++;
                    player.sprite.setFrame(player.currentRotation);
                }
                player.position.y -= speed;
            } 
            else if (movingDown) {
                if (player.currentRotation > 0) {
                    player.currentRotation--;
                    player.sprite.setFrame(player.currentRotation);
                }
                player.position.y += speed;
            } 
            else {
                if (player.currentRotation < 2) {
                    player.currentRotation++;
                    player.sprite.setFrame(player.currentRotation);
                }
                else if (player.currentRotation > 2) {
                    player.currentRotation--;
                    player.sprite.setFrame(player.currentRotation);
                }
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                player.position.x -= speed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                player.position.x += speed;
            }

            if (player.position.x < 0)
                player.position.x = 0;
            if (player.position.x > WINDOW_WIDTH - spriteSize.x)
                player.position.x = WINDOW_WIDTH - spriteSize.x;

            if (player.position.y < 0)
                player.position.y = 0;
            if (player.position.y > WINDOW_HEIGHT - spriteSize.y)
                player.position.y = WINDOW_HEIGHT - spriteSize.y;

            player.sprite.setPosition(player.position.x, player.position.y);
            
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && shootCooldown <= 0.0f) {
                LocalProjectile projectile;
                projectile.id = nextProjectileId++;
                
                projectile.position = sf::Vector2f(
                    player.position.x + spriteSize.x,
                    player.position.y + spriteSize.y / 2.0f - 8.0f
                );
                
                projectile.velocity = sf::Vector2f(500.0f, 0.0f);
                
                if (auto* tex = rm.getTexture("projectiles_sheet")) {
                    projectile.sprite.setAnimation(
                tex,
                sf::Vector2u(22.8, 22.8),
                3,
                0,
                0.05f
            );
                    projectile.sprite.play();
                    projectile.sprite.setLoop(true);
                    projectile.sprite.setScale(2.0f, 2.0f);
                    projectile.sprite.setPosition(projectile.position.x, projectile.position.y);
                }
                
                localProjectiles.push_back(std::move(projectile));
                shootCooldown = SHOOT_DELAY;
                
                std::cout << "Piou piou piou! (Local projectile " << (nextProjectileId - 1) << ")\n";
            }
        }
        
        for (auto& [id, animSprite] : projectileSprites) {
            animSprite.update(deltaTime);
        }
        
        for (auto& projectile : localProjectiles) {
            projectile.position += projectile.velocity * deltaTime;
            projectile.sprite.setPosition(projectile.position.x, projectile.position.y);
            projectile.sprite.update(deltaTime);
        }
        
        localProjectiles.erase(
            std::remove_if(localProjectiles.begin(), localProjectiles.end(),
                [](const LocalProjectile& p) {
                    return p.position.x > WINDOW_WIDTH + 50 || p.position.x < -50 ||
                           p.position.y > WINDOW_HEIGHT + 50 || p.position.y < -50;
                }),
            localProjectiles.end()
        );

        _entityManager->update(deltaTime);

        window.clear();
        _entityManager->render(window.getWindow());

        for (const auto& player : players) {
            if (player.active) {
                player.sprite.draw(window.getWindow());
            }
        }

        if (!projectileSprites.empty()) {
            std::cout << "[Render] Drawing " << projectileSprites.size() << " projectiles\n";
            for (auto& [id, animSprite] : projectileSprites) {
                animSprite.draw(window.getWindow());
            }
        }
        
        for (auto& projectile : localProjectiles) {
            projectile.sprite.draw(window.getWindow());
        }

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