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

enum class KeyCode : uint8_t {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
    SHOOT = 4
};

enum class InputAction : uint8_t {
    PRESSED = 1,
    RELEASED = 0
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
        if (eventType == 0) { // JOIN
            _incomingMessages.push("PLAYER_JOIN:" + std::to_string(playerId));
        } else if (eventType == 1) { // LEAVE
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

void CLIENT::Core::graphicsLoop()
{
    CLIENT::Window window("R-Type Client", WINDOW_WIDTH, WINDOW_HEIGHT);
    auto& rm = ResourceManager::getInstance();

    struct Player {
        AnimatedSprite sprite;
        int playerId;
        bool active;
        sf::Vector2f position;
        int currentRotation;
    };

    std::array<Player, 4> players;
    
    if (auto* tex = rm.getTexture("player_ships")) {
        for (int i = 0; i < 4; ++i) {
            players[i].playerId = i;
            players[i].active = false;
            players[i].currentRotation = 0;
            
            players[i].sprite.setAnimation(
                tex,
                sf::Vector2u(33.2, 17.2),
                4,
                i,
                0.1f 
            );
            players[i].sprite.pause();
            players[i].sprite.setFrame(0);
            
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

    while (window.isOpen() && _running) {
        float deltaTime = window.getDeltaTime();
        
        {
            std::lock_guard<std::mutex> lock(_incomingMutex);
            while (!_incomingMessages.empty()) {
                std::string msg = _incomingMessages.front();
                _incomingMessages.pop();
                
                std::cout << "[Graphics] Received from network: " << msg << "\n";
                
                if (msg.find("PLAYER_ID:") == 0) {
                    _myPlayerId = std::stoi(msg.substr(10));
                    if (_myPlayerId < 4) {
                        players[_myPlayerId].active = true;
                        std::cout << "*** Assigned Player ID: " << int(_myPlayerId) << " ***\n";
                        std::cout << "*** Using ship " << int(_myPlayerId) + 1 << "/4 ***\n";
                    }
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
            }
        }

        window.pollEvents();
        
        if (_myPlayerId >= 4) {
            window.clear();
            
            for (const auto& player : players) {
                if (player.active) {
                    player.sprite.draw(window.getWindow());
                }
            }
            
            window.display();
            continue;
        }
        
        const auto& actions = window.getPendingActions();
        
        for (const auto& action : actions) {
            if (!keyStates[action]) {
                keyStates[action] = true;
                
                std::string inputMsg;
                if (action == "MOVE_UP") {
                    inputMsg = "INPUT:" + std::to_string(static_cast<uint8_t>(KeyCode::UP)) + ":" 
                             + std::to_string(static_cast<uint8_t>(InputAction::PRESSED));
                } else if (action == "MOVE_DOWN") {
                    inputMsg = "INPUT:" + std::to_string(static_cast<uint8_t>(KeyCode::DOWN)) + ":" 
                             + std::to_string(static_cast<uint8_t>(InputAction::PRESSED));
                } else if (action == "MOVE_LEFT") {
                    inputMsg = "INPUT:" + std::to_string(static_cast<uint8_t>(KeyCode::LEFT)) + ":" 
                             + std::to_string(static_cast<uint8_t>(InputAction::PRESSED));
                } else if (action == "MOVE_RIGHT") {
                    inputMsg = "INPUT:" + std::to_string(static_cast<uint8_t>(KeyCode::RIGHT)) + ":" 
                             + std::to_string(static_cast<uint8_t>(InputAction::PRESSED));
                } else if (action == "SHOOT") {
                    inputMsg = "INPUT:" + std::to_string(static_cast<uint8_t>(KeyCode::SHOOT)) + ":" 
                             + std::to_string(static_cast<uint8_t>(InputAction::PRESSED));
                    std::cout << "Piou piou piou\n";
                }
                
                if (!inputMsg.empty()) {
                    std::lock_guard<std::mutex> lock(_outgoingMutex);
                    _outgoingMessages.push(inputMsg);
                }
            }
        }
        
        std::vector<std::string> allKeys = {"MOVE_UP", "MOVE_DOWN", "MOVE_LEFT", "MOVE_RIGHT", "SHOOT"};
        for (const auto& key : allKeys) {
            bool isPressed = std::find(actions.begin(), actions.end(), key) != actions.end();
            
            if (keyStates[key] && !isPressed) {
                keyStates[key] = false;
                
                std::string inputMsg;
                if (key == "MOVE_UP") {
                    inputMsg = "INPUT:" + std::to_string(static_cast<uint8_t>(KeyCode::UP)) + ":" 
                             + std::to_string(static_cast<uint8_t>(InputAction::RELEASED));
                } else if (key == "MOVE_DOWN") {
                    inputMsg = "INPUT:" + std::to_string(static_cast<uint8_t>(KeyCode::DOWN)) + ":" 
                             + std::to_string(static_cast<uint8_t>(InputAction::RELEASED));
                } else if (key == "MOVE_LEFT") {
                    inputMsg = "INPUT:" + std::to_string(static_cast<uint8_t>(KeyCode::LEFT)) + ":" 
                             + std::to_string(static_cast<uint8_t>(InputAction::RELEASED));
                } else if (key == "MOVE_RIGHT") {
                    inputMsg = "INPUT:" + std::to_string(static_cast<uint8_t>(KeyCode::RIGHT)) + ":" 
                             + std::to_string(static_cast<uint8_t>(InputAction::RELEASED));
                } else if (key == "SHOOT") {
                    inputMsg = "INPUT:" + std::to_string(static_cast<uint8_t>(KeyCode::SHOOT)) + ":" 
                             + std::to_string(static_cast<uint8_t>(InputAction::RELEASED));
                }
                
                if (!inputMsg.empty()) {
                    std::lock_guard<std::mutex> lock(_outgoingMutex);
                    _outgoingMessages.push(inputMsg);
                }
            }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            if (players[_myPlayerId].currentRotation < 3) {
                players[_myPlayerId].currentRotation++;
                players[_myPlayerId].sprite.setFrame(players[_myPlayerId].currentRotation);
            }
            players[_myPlayerId].position.y -= 200.0f * deltaTime;
            players[_myPlayerId].sprite.setPosition(players[_myPlayerId].position.x, 
                                                   players[_myPlayerId].position.y);
        } 
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            if (players[_myPlayerId].currentRotation > 0) {
                players[_myPlayerId].currentRotation--;
                players[_myPlayerId].sprite.setFrame(players[_myPlayerId].currentRotation);
            }
            players[_myPlayerId].position.y += 200.0f * deltaTime;
            players[_myPlayerId].sprite.setPosition(players[_myPlayerId].position.x, 
                                                   players[_myPlayerId].position.y);
        }
        else {
            if (players[_myPlayerId].currentRotation > 0) {
                players[_myPlayerId].currentRotation--;
                players[_myPlayerId].sprite.setFrame(players[_myPlayerId].currentRotation);
            }
        }
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            players[_myPlayerId].position.x -= 200.0f * deltaTime;
            players[_myPlayerId].sprite.setPosition(players[_myPlayerId].position.x, 
                                                   players[_myPlayerId].position.y);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            players[_myPlayerId].position.x += 200.0f * deltaTime;
            players[_myPlayerId].sprite.setPosition(players[_myPlayerId].position.x, 
                                                   players[_myPlayerId].position.y);
        }

        window.clear();
        
        for (const auto& player : players) {
            if (player.active) {
                player.sprite.draw(window.getWindow());
            }
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