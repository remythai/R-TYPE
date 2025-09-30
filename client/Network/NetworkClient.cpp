/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkClient.cpp - VERSION UDP
*/

#include "NetworkClient.hpp"
#include <iostream>

CLIENT::NetworkClient::NetworkClient(const std::string& host, unsigned short port)
    : _socket(_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0))
{
    try {
        asio::ip::udp::resolver resolver(_ioContext);
        asio::error_code ec;

        auto endpoints = resolver.resolve(asio::ip::udp::v4(), host, std::to_string(port), ec);
        if (ec) {
            throw NetworkError("Failed to resolve host '" + host + "': " + ec.message());
        }

        _serverEndpoint = *endpoints.begin();

        _socket.non_blocking(true, ec);
        if (ec) {
            throw NetworkError("Failed to set socket to non-blocking mode: " + ec.message());
        }

        std::cout << "UDP client connected to " << host << ":" << port << "\n";

    } catch (const std::exception &e) {
        throw NetworkError(std::string("NetworkClient initialization failed: ") + e.what());
    }
}

void CLIENT::NetworkClient::sendMessage(const std::string& msg)
{
    asio::error_code ec;
    _socket.send_to(asio::buffer(msg), _serverEndpoint, 0, ec);
    if (ec) {
        throw NetworkError("Failed to send message: " + ec.message());
    }
}

std::string CLIENT::NetworkClient::receiveMessage()
{
    char data[1024];
    asio::error_code ec;
    asio::ip::udp::endpoint sender_endpoint;

    size_t length = _socket.receive_from(asio::buffer(data), sender_endpoint, 0, ec);
    
    if (ec == asio::error::would_block) {
        return "";
    }
    
    if (ec) {
        throw NetworkError("Failed to receive message: " + ec.message());
    }

    return std::string(data, length);
}
