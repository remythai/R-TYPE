/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** handleClient.cpp
*/

#include <iostream>

#include "NetworkServer.hpp"

uint8_t rtype::NetworkServer::findPlayerIdByEndpoint(
    const asio::ip::udp::endpoint& endpoint)
{
    std::lock_guard<std::mutex> lock(_playerSlotsMutex);
    for (const auto& slot : _playerSlots)
        if (slot.isUsed && slot.endpoint == endpoint)
            return slot.playerId;
    return 255;
}

void rtype::NetworkServer::handleJoinPacket(
    const asio::ip::udp::endpoint& clientEndpoint,
    const std::vector<uint8_t>& payload)
{
    std::string username(payload.begin(), payload.end());
    std::cout << "[Username=" << username << "]";

    std::lock_guard<std::mutex> lock(_clientsMutex);

    for (const auto& slot : _playerSlots) {
        if (slot.isUsed && slot.endpoint == clientEndpoint) {
            std::cout << " [Already connected as Player " << int(slot.playerId)
                      << "]" << std::endl;
            sendPlayerIdAssignment(clientEndpoint, slot.playerId);
            return;
        }
    }

    if (countActivePlayers() >= 4) {
        std::cout << " -> Connection refused: server full" << std::endl;

        std::vector<uint8_t> fullPacket = {
            static_cast<uint8_t>(rtype::PacketType::PLAYER_ID_ASSIGNMENT),
            0,
            0,
            0,
            0,
            0,
            0,
            255};
        _socket.send_to(asio::buffer(fullPacket), clientEndpoint);
        return;
    }

    uint8_t assignedPlayerId = 255;
    for (auto& slot : _playerSlots) {
        if (!slot.isUsed) {
            slot.isUsed = true;
            slot.endpoint = clientEndpoint;
            slot.username = username;
            slot.lastActive = std::chrono::steady_clock::now();
            assignedPlayerId = slot.playerId;

            slot.entity = createPlayerEntity(assignedPlayerId);

            break;
        }
    }

    int clientId = _nextClientId++;
    _clients[clientId] = clientEndpoint;

    std::cout << " -> Assigned Player ID " << int(assignedPlayerId)
              << ", Total players: " << countActivePlayers() << "/4"
              << std::endl;

    sendPlayerIdAssignment(clientEndpoint, assignedPlayerId);
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
                  << "][KeyCode=" << int(keyCode) << "][Action=" << int(action)
                  << "]";

        uint8_t expectedPlayerId = findPlayerIdByEndpoint(clientEndpoint);
        if (expectedPlayerId != playerId) {
            std::cout << " [WARNING: PlayerId mismatch! Expected "
                      << int(expectedPlayerId) << "]";
        } else {
            applyInputToEntity(playerId, keyCode, action);
            std::cout << " [Input applied to ECS]";
        }
    }
}

void rtype::NetworkServer::handleSnapshotPacket(
    const asio::ip::udp::endpoint& clientEndpoint,
    const std::vector<uint8_t>& payload)
{
    if (!payload.empty())
        std::cout << "[NbEntities=" << int(payload[0]) << "][EntityData="
                  << std::string(payload.begin() + 1, payload.end()) << "]";
}

void rtype::NetworkServer::handleClientPacket(
    const asio::ip::udp::endpoint& clientEndpoint, PacketType type,
    uint16_t packetId, uint32_t timestamp, const std::vector<uint8_t>& payload)
{
    std::cout << "[SERVER] From " << clientEndpoint << " -> ";
    std::cout << "[Type=" << packetTypeToString(type) << "]";
    std::cout << "[PacketId=" << packetId << "]";
    std::cout << "[Timestamp=" << timestamp << "]";

    uint8_t playerId = findPlayerIdByEndpoint(clientEndpoint);
    if (playerId != 255)
        _playerSlots[playerId].lastActive = std::chrono::steady_clock::now();

    switch (type) {
        case PacketType::JOIN:
            handleJoinPacket(clientEndpoint, payload);
            break;

        case PacketType::INPUT:
            handleInputPacket(clientEndpoint, payload);
            break;

        case PacketType::SNAPSHOT:
            handleSnapshotPacket(clientEndpoint, payload);
            break;

        default:
            std::cout << "[Unknown packet]";
            break;
    }

    std::cout << std::endl;
}