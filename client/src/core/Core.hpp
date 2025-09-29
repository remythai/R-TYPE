/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.hpp
*/

#pragma once

#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include "../macros.hpp"
#include "../network/NetworkClient.hpp"
#include "../graphics/AnimatedSprite.hpp"
#include "../graphics/ResourceManager.hpp"

namespace CLIENT {
    class Core {
        public:
            class CoreError : public std::exception {
                private:
                    std::string _message;
                public:
                    CoreError(std::string  message) : _message(std::move(message)) {}
                    [[nodiscard]] const char* what() const noexcept override { return _message.c_str(); }
            };

            Core(char **argv);
            ~Core();

            void run();


        private:
            void networkLoop();
            void graphicsLoop();

            void loadResources();

            std::unique_ptr<NetworkClient> _networkClient;
            std::string _hostname;
            int _port;

            std::thread _networkThread;
            std::atomic<bool> _running;
            
            std::queue<std::string> _incomingMessages;
            std::mutex _incomingMutex;
            
            std::queue<std::string> _outgoingMessages;
            std::mutex _outgoingMutex;
    };
} // namespace CLIENT

int execute_rtypeClient(char **argv);
