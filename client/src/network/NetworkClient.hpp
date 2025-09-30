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
#include <vector>
#include <cstdint>
#include <iostream>
#include <functional>
#include "../../../server/src/network/NetworkServer.hpp"

class NetworkClient {
public:
    NetworkClient(const std::string& host, unsigned short port);

    void sendInput(uint8_t playerId, uint8_t keyCode, uint8_t action);
    void sendJoin(const std::string& username);
    void sendPing(uint16_t packetId);

    void startReceiving();

private:
    asio::io_context _ioContext;
    asio::ip::udp::socket _socket;
    asio::ip::udp::endpoint _serverEndpoint;

    std::vector<uint8_t> _recvBuffer;

    void sendPacket(rtype::PacketType type, uint16_t packetId, uint32_t timestamp,
                    const std::vector<uint8_t>& payload);

    void doReceive();
    void handlePacket(const std::vector<uint8_t>& buffer, size_t bytesReceived,
                    const asio::ip::udp::endpoint& sender);
};
