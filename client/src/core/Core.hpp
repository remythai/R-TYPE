/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.hpp
*/

#pragma once

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include "../network/NetworkClient.hpp"
#include "../macros.hpp"
#include <SFML/Audio/Music.hpp>

namespace CLIENT {

class Core {
public:
    class CoreError : public std::exception {
    private:
        std::string _message;
    public:
        explicit CoreError(const std::string& msg) : _message(msg) {}
        const char* what() const noexcept override { return _message.c_str(); }
    };

    Core(char **argv);
    ~Core();
    
    void run();

private:
    void loadResources();
    void networkLoop();
    void graphicsLoop();

    std::string _hostname;
    unsigned short _port;
    std::string _username;
    uint8_t _myPlayerId;
    
    std::unique_ptr<NetworkClient> _networkClient;
    
    std::thread _networkThread;
    bool _running;
    
    std::queue<std::string> _incomingMessages;
    std::queue<std::string> _outgoingMessages;
    std::mutex _incomingMutex;
    std::mutex _outgoingMutex;

    std::unique_ptr<sf::Music> _backgroundMusic;

};

} // namespace CLIENT

int execute_rtypeClient(char **argv);
