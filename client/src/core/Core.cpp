#include "Core.hpp"
#include "../graphics/Window.hpp"
#include "../graphics/ResourceManager.hpp"
#include "../graphics/AnimatedSprite.hpp"
#include <iostream>
#include <memory>
#include <chrono>
#include <array>
#include <thread>

CLIENT::Core::Core(char **argv)
    : _port(0), _running(false)
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
    _networkClient->sendJoin(_username);
    _networkClient->startReceiving();

    std::cout << "[Core] Client initialized with username: " << _username << "\n";
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
    
    std::cout << "✅ Resources loaded\n";
}

void CLIENT::Core::networkLoop()
{
    std::cout << "[Network Thread] Started\n";

    while (_running) {
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
            players[i].position = sf::Vector2f(100.0f, 100.0f + i * 100.0f);
            players[i].sprite.setPosition(players[i].position.x, players[i].position.y);
            players[i].sprite.setScale(2.0f, 2.0f);
        }
        
        // Activer tous les joueurs pour le test visuel
        for (int i = 0; i < 4; ++i) {
            players[i].active = true;
        }
    }

    while (window.isOpen() && _running) {
        float deltaTime = window.getDeltaTime();
        {
            std::lock_guard<std::mutex> lock(_incomingMutex);
            while (!_incomingMessages.empty()) {
                std::string msg = _incomingMessages.front();
                _incomingMessages.pop();
                
                std::cout << "[Graphics] Received from network: " << msg << "\n";
                
                if (msg.find("PLAYER_JOIN:") == 0) {
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
                    
                    if (playerId >= 0 && playerId < 4) {
                        players[playerId].position = sf::Vector2f(x, y);
                        players[playerId].sprite.setPosition(x, y);
                    }
                }
            }
        }

        window.pollEvents();
        
        const auto& actions = window.getPendingActions();
        
        bool movingUp = false;
        bool movingDown = false;
        
        for (const auto& action : actions) {
            if (action == "MOVE_UP") {
                movingUp = true;
            } else if (action == "MOVE_DOWN") {
                movingDown = true;
            } else if (action == "MOVE_LEFT") {
                players[0].position.x -= 5.0f;
                players[0].sprite.setPosition(players[0].position.x, players[0].position.y);
            } else if (action == "MOVE_RIGHT") {
                players[0].position.x += 5.0f;
                players[0].sprite.setPosition(players[0].position.x, players[0].position.y);
            } else if (action == "SHOOT") {
                std::cout << "Piou piou piou\n";
            }
        }
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            if (players[0].currentRotation < 3) {
                players[0].currentRotation++;
                players[0].sprite.setFrame(players[0].currentRotation);
            }
            players[0].position.y -= 200.0f * deltaTime;
            players[0].sprite.setPosition(players[0].position.x, players[0].position.y);
        } 
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            if (players[0].currentRotation > 0) {
                players[0].currentRotation--;
                players[0].sprite.setFrame(players[0].currentRotation);
            }
            players[0].position.y += 200.0f * deltaTime;
            players[0].sprite.setPosition(players[0].position.x, players[0].position.y);
        }
        else {
            if (players[0].currentRotation > 0) {
                players[0].currentRotation--;
                players[0].sprite.setFrame(players[0].currentRotation);
            }
        }
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            players[0].position.x -= 200.0f * deltaTime;
            players[0].sprite.setPosition(players[0].position.x, players[0].position.y);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            players[0].position.x += 200.0f * deltaTime;
            players[0].sprite.setPosition(players[0].position.x, players[0].position.y);
        }
        
        if (!actions.empty()) {
            std::lock_guard<std::mutex> lock(_outgoingMutex);
            for (const auto& action : actions) {
                _outgoingMessages.push(action);
            }
        window.pollEvents();

        static int frame = 0;
        if (++frame % 60 == 0) {
            _networkClient->sendInput(1, 1, 1);
            std::cout << "[Graphics] Sent simulated INPUT\n";
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
