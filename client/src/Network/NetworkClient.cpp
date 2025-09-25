/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkClient.cpp
*/

#include "NetworkClient.hpp"

NetworkClient::NetworkClient(const std::string& host, unsigned short port)
    : _socket(_ioContext)
{
    asio::ip::tcp::resolver resolver(_ioContext);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    asio::connect(_socket, endpoints);
}

void NetworkClient::sendMessage(const std::string& msg)
{
    _socket.write_some(asio::buffer(msg));
}

std::string NetworkClient::receiveMessage()
{
    char data[1024];

    size_t length = _socket.read_some(asio::buffer(data));
    return std::string(data, length);
}
