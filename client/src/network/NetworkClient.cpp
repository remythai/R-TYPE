/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkClient.cpp
*/

#include "NetworkClient.hpp"
#include <iostream>

CLIENT::NetworkClient::NetworkClient(const std::string& host, unsigned short port)
    : _socket(_ioContext)
{
    try {
        asio::ip::tcp::resolver resolver(_ioContext);
        asio::error_code ec;

        auto endpoints = resolver.resolve(host, std::to_string(port), ec);
            if (ec) {
                throw NetworkError("Failed to resolve host '" + host + "': " + ec.message());
        }

        asio::connect(_socket, endpoints, ec);
        if (ec) {
            throw NetworkError("Failed to connect to " + host + ":" + std::to_string(port) + " -> " + ec.message());
        }

        } catch (const std::exception &e) {
            throw NetworkError(std::string("NetworkClient initialization failed: ") + e.what());
    }
}

    void CLIENT::NetworkClient::sendMessage(const std::string& msg)
    {
        asio::error_code ec;
        _socket.write_some(asio::buffer(msg), ec);
        if (ec) {
            throw NetworkError("Failed to send message: " + ec.message());
        }
    }

    std::string CLIENT::NetworkClient::receiveMessage()
    {
        char data[1024];
        asio::error_code ec;

        size_t length = _socket.read_some(asio::buffer(data), ec);
        if (ec && ec != asio::error::eof) {
            throw NetworkError("Failed to receive message: " + ec.message());
        }

        return std::string(data, length);
    }

