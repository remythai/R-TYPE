/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkClient.hpp
*/

#pragma once

#include <asio.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "../../../server/src/network/NetworkServer.hpp"

class NetworkClient
{
   public:
    NetworkClient(const std::string& host, unsigned short port);

    void sendPacket(
        rtype::PacketType type, uint16_t packetId, uint32_t timestamp,
        const std::vector<uint8_t>& payload);
    void sendInput(uint8_t playerId, uint8_t keyCode, uint8_t action);
    void sendJoin(const std::string& username);
    void startReceiving();

    void setOnPlayerIdReceived(std::function<void(uint8_t)> callback);
    void setOnPlayerEvent(std::function<void(uint8_t, uint8_t)> callback);
    void setOnSnapshot(std::function<void(int score, const std::vector<uint8_t>&)> callback);

    void setOnTimeout(std::function<void(uint8_t)> callback);
    void setOnKilled(std::function<void(uint8_t)> callback);

   private:
    void doReceive();
    void handlePacket(
        const std::vector<uint8_t>& buffer, size_t bytesReceived,
        const asio::ip::udp::endpoint& sender);

    asio::io_context _ioContext;
    asio::ip::udp::socket _socket;
    asio::ip::udp::endpoint _serverEndpoint;
    std::vector<uint8_t> _recvBuffer;

    std::function<void(uint8_t)> _onPlayerIdReceived;
    std::function<void(uint8_t, uint8_t)> _onPlayerEvent;
    std::function<void(int score, const std::vector<uint8_t>&)> _onSnapshot;

    std::function<void(uint8_t)> _onTimeout;
    std::function<void(uint8_t)> _onKilled;
};