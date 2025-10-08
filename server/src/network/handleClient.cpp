/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** handleClient.cpp
*/

#include "NetworkServer.hpp"
#include <iostream>

uint8_t rtype::NetworkServer::findPlayerIdByEndpoint(const asio::ip::udp::endpoint& endpoint)
{
    for (const auto& slot : _playerSlots)
        if (slot.isUsed && slot.endpoint == endpoint)
            return slot.playerId;
    return 255;
}

void rtype::NetworkServer::handleInputPacket(
    const asio::ip::udp::endpoint& clientEndpoint,
    const std::vector<uint8_t>& payload)
{
    if (payload.size() >= 3) {
        uint8_t playerId = payload[0];
        uint8_t keyCode = payload[1];
        uint8_t action = payload[2];

        std::cout << "[PlayerId=" << int(playerId)
                << "][KeyCode=" << int(keyCode)
                << "][Action=" << int(action) << "]";

        uint8_t expectedPlayerId = findPlayerIdByEndpoint(clientEndpoint);
        if (expectedPlayerId != playerId) {
            std::cout << " [WARNING: PlayerId mismatch! Expected "
                    << int(expectedPlayerId) << "]";
        }
    }
}

void rtype::NetworkServer::handlePingPacket(
    const asio::ip::udp::endpoint& clientEndpoint,
    uint16_t packetId, uint32_t timestamp)
{
    std::cout << "[PacketId=" << packetId << "]";
    {
        auto response = serializePingResponse(packetId, timestamp);
        _socket.send_to(asio::buffer(response), clientEndpoint);
    }
}

void rtype::NetworkServer::handleSnapshotPacket(
    const asio::ip::udp::endpoint& clientEndpoint,
    const std::vector<uint8_t>& payload)
{
    if (!payload.empty())
        std::cout << "[NbEntities=" << int(payload[0])
                << "][EntityData=" << std::string(payload.begin()+1, payload.end()) << "]";
}

void rtype::NetworkServer::handleEntityEventPacket(
    const asio::ip::udp::endpoint& clientEndpoint,
    const std::vector<uint8_t>& payload)
{
    if (payload.size() >= 2)
        std::cout << "[EntityId=" << int(payload[0])
                << "][EventType=" << int(payload[1])
                << "][ExtraData=" << (payload.size() > 2 ? std::string(payload.begin()+2, payload.end()) : "") << "]";
}

void rtype::NetworkServer::handlePlayerEventPacket(
    const asio::ip::udp::endpoint& clientEndpoint,
    const std::vector<uint8_t>& payload)
{
    if (payload.size() >= 2)
        std::cout << "[PlayerId=" << int(payload[0])
                << "][EventType=" << int(payload[1])
                << "][Score=" << (payload.size() > 2 ? int(payload[2]) : 0) << "]";
}

void rtype::NetworkServer::handleClientPacket(
    const asio::ip::udp::endpoint& clientEndpoint,
    PacketType type, uint16_t packetId, uint32_t timestamp,
    const std::vector<uint8_t>& payload)
{
    std::cout << "[SERVER] From " << clientEndpoint << " -> ";
    std::cout << "[Type=" << packetTypeToString(type) << "]";
    std::cout << "[PacketId=" << packetId << "]";
    std::cout << "[Timestamp=" << timestamp << "]";

    switch(type) {
        case PacketType::JOIN:
            handleJoinPacket(clientEndpoint, payload);
            break;

        case PacketType::INPUT:
            handleInputPacket(clientEndpoint, payload);
            break;

        case PacketType::PING:
            handlePingPacket(clientEndpoint, packetId, timestamp);
            break;

        case PacketType::SNAPSHOT:
            handleSnapshotPacket(clientEndpoint, payload);
            break;

        case PacketType::ENTITY_EVENT:
            handleEntityEventPacket(clientEndpoint, payload);
            break;

        case PacketType::PLAYER_EVENT:
            handlePlayerEventPacket(clientEndpoint, payload);
            break;

        case PacketType::PING_RESPONSE:
            std::cout << "[PacketId=" << packetId << "]";
            break;

        default:
            std::cout << "[Unknown packet]";
            break;
    }

    std::cout << std::endl;
}