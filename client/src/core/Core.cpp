/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.cpp
*/

#include "Core.hpp"
#include "../graphics/Window.hpp"
#include <iostream>
#include <memory>
#include <chrono>

CLIENT::Core::Core(char **argv)
    : _port(0), _running(false)
{
    for (int i = 1; i <= 3; ++i) {
        std::string arg = argv[i];
        if (arg == "-p") {
            try {
                _port = std::stoi(argv[i + 1]);
            } catch (const std::invalid_argument &) {
                throw CoreError("Invalid port: not a number");
            } catch (const std::out_of_range &) {
                throw CoreError("Invalid port: number out of range");
            }
        } else if (arg == "-h")
            _hostname = argv[i + 1];
    }

    if (_port == 0 || _hostname.empty())
        throw CoreError("Missing -p or -h argument");
    
    _networkClient = std::make_unique<NetworkClient>(_hostname, _port);
    _networkClient->sendMessage("connected");

    std::cout << "aujourd'hui je suis\n";
}

CLIENT::Core::~Core()
{
    _running = false;
    
    if (_networkThread.joinable()) {
        _networkThread.join();
    }
    
    std::cout << "je t'ai regardé dormir\n";
}

void CLIENT::Core::run()
{
    _running = true;
    
    _networkThread = std::thread(&Core::networkLoop, this);
    graphicsLoop();
    _running = false;
    if (_networkThread.joinable()) {
        _networkThread.join();
    }
}

void CLIENT::Core::networkLoop()
{
    std::cout << "[Network Thread] Started\n";
    
    while (_running) {
        try {
            {
                std::lock_guard<std::mutex> lock(_outgoingMutex);
                while (!_outgoingMessages.empty()) {
                    std::string msg = _outgoingMessages.front();
                    _outgoingMessages.pop();
                    
                    std::cout << "[Network] Sending: " << msg << "\n";
                    _networkClient->sendMessage(msg);
                }
            }
            
            std::string received = _networkClient->receiveMessage();
            if (!received.empty()) {
                std::lock_guard<std::mutex> lock(_incomingMutex);
                _incomingMessages.push(received);
                std::cout << "[Network] Received: " << received << "\n";
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
        } catch (const NetworkClient::NetworkError &e) {
            std::cerr << "[Network Thread] Error: " << e.what() << "\n";
        }
    }
    
    std::cout << "[Network Thread] Stopped\n";
}

void CLIENT::Core::graphicsLoop()
{
    CLIENT::Window window("R-Type Client", WINDOW_WIDTH, WINDOW_HEIGHT);

    while (window.isOpen() && _running) {
        {
            std::lock_guard<std::mutex> lock(_incomingMutex);
            while (!_incomingMessages.empty()) {
                std::string msg = _incomingMessages.front();
                _incomingMessages.pop();
                
                std::cout << "[Graphics] Received from network: " << msg << "\n";
            }
        }

        window.pollEvents();
        for (const auto& action : window.getPendingActions()) {
            std::lock_guard<std::mutex> lock(_outgoingMutex);
            _outgoingMessages.push(action);
            std::cout << "[Graphics] Sending action: " << action << "\n";
        }

        window.clear();
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
    } catch (const CLIENT::NetworkClient::NetworkError &error) {
        std::cerr << "Network error: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}