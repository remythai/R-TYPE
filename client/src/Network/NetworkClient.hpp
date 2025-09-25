/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkClient.hpp
*/

#pragma once
#include <asio.hpp>

class NetworkClient {
    public:
        NetworkClient(const std::string& host, unsigned short port);
        void sendMessage(const std::string& msg);
        std::string receiveMessage();

    private:
        asio::io_context _ioContext;
        asio::ip::tcp::socket _socket;
};
