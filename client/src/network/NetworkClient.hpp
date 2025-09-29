/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkClient.hpp
*/

#pragma once
#include <asio.hpp>

namespace CLIENT {
    class NetworkClient {
        public:
            NetworkClient(const std::string& host, unsigned short port);
            class NetworkError : public std::exception {
                private:
                    std::string _message;
                public:
                    NetworkError(std::string  message) : _message(std::move(message)) {}
                    [[nodiscard]] const char* what() const noexcept override { return _message.c_str(); }
            };
            
            void sendMessage(const std::string& msg);
            std::string receiveMessage();

        private:
            asio::io_context _ioContext;
            asio::ip::tcp::socket _socket;
    };
}
