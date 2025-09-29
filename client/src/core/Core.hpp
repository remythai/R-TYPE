/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.hpp
*/

#pragma once

#include "../macros.hpp"
#include <exception>
#include <memory>
#include <string>
#include <utility>
#include "../network/NetworkClient.hpp"

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
            std::unique_ptr<NetworkClient> _networkClient;

            std::string _hostname;

            int _port;
    };
} // namespace CLIENT

int execute_rtypeClient(char **argv);
