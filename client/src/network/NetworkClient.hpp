/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkClient.hpp - VERSION UDP
*/

#pragma once

#include <string>
#include <exception>
#include <asio.hpp>

namespace CLIENT {
    class NetworkClient {
        public:
            class NetworkError : public std::exception {
                private:
                    std::string _message;
                public:
                    explicit NetworkError(std::string message) : _message(std::move(message)) {}
                    [[nodiscard]] const char* what() const noexcept override { return _message.c_str(); }
            };

            NetworkClient(const std::string &host, unsigned short port);
            
            void sendMessage(const std::string &msg);
            std::string receiveMessage();

        private:
            asio::io_context _ioContext;
            asio::ip::udp::socket _socket;
            asio::ip::udp::endpoint _serverEndpoint;
    };
}
