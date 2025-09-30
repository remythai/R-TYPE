/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkServer.cpp
*/

#include "NetworkServer.hpp"
#include <thread>
#include <iostream>
#include <algorithm>

template<typename T>
std::vector<uint8_t> toBytes(T value) {
    std::vector<uint8_t> bytes(sizeof(T));
    for (size_t i = 0; i < sizeof(T); ++i)
        bytes[sizeof(T) - 1 - i] = (value >> (i * 8)) & 0xFF;
    return bytes;
}

template<typename T>
T fromBytes(const uint8_t* data) {
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i)
        value |= data[i] << (8 * (sizeof(T) - 1 - i));
    return value;
}

rtype::NetworkServer::NetworkServer(unsigned short port, std::string const &hostname)
    : _socket(_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), 
      _running(false), 
      _hostname(hostname)
{
    for (int i = 0; i < 4; ++i) {
        _playerSlots[i].isUsed = false;
        _playerSlots[i].playerId = i;
    }
}

rtype::NetworkServer::~NetworkServer() {
    stop();
}

void rtype::NetworkServer::run() {
    _running = true;
    doReceive();
    std::cout << "UDP Server running..." << std::endl;
    _ioContext.run();
}

void rtype::NetworkServer::stop() {
    _running = false;
    _ioContext.stop();
    std::lock_guard<std::mutex> lock(_clientsMutex);
    _clients.clear();
    for (auto& slot : _playerSlots) {
        slot.isUsed = false;
    }
    std::cout << "Server stopped." << std::endl;
}

void rtype::NetworkServer::doReceive() {
    auto buffer = std::make_shared<std::vector<uint8_t>>(1024);
    auto clientEndpoint = std::make_shared<asio::ip::udp::endpoint>();

    _socket.async_receive_from(
        asio::buffer(*buffer), *clientEndpoint,
        [this, buffer, clientEndpoint](std::error_code ec, std::size_t bytesReceived) {
            if (!ec && bytesReceived >= 7) {
                PacketType type = static_cast<PacketType>((*buffer)[0]);
                uint16_t packetId = fromBytes<uint16_t>(buffer->data() + 1);
                uint32_t timestamp = fromBytes<uint32_t>(buffer->data() + 3);

                size_t payloadSize = bytesReceived - 7;
                std::vector<uint8_t> payload(payloadSize);
                std::copy(buffer->begin() + 7, buffer->begin() + 7 + payloadSize, payload.begin());

                handleClientPacket(*clientEndpoint, type, packetId, timestamp, payload);
            }
            if (_running)
                doReceive();
        }
    );
}

std::string rtype::NetworkServer::packetTypeToString(rtype::PacketType type)
{
    switch(type) {
        case rtype::PacketType::INPUT: return "INPUT";
        case rtype::PacketType::JOIN: return "JOIN";
        case rtype::PacketType::PING: return "PING";
        case rtype::PacketType::SNAPSHOT: return "SNAPSHOT";
        case rtype::PacketType::ENTITY_EVENT: return "ENTITY_EVENT";
        case rtype::PacketType::PLAYER_EVENT: return "PLAYER_EVENT";
        case rtype::PacketType::PING_RESPONSE: return "PING_RESPONSE";
        case rtype::PacketType::PLAYER_ID_ASSIGNMENT: return "PLAYER_ID_ASSIGNMENT";
        default: return "UNKNOWN";
    }
}

uint8_t rtype::NetworkServer::findPlayerIdByEndpoint(const asio::ip::udp::endpoint& endpoint) {
    for (const auto& slot : _playerSlots) {
        if (slot.isUsed && slot.endpoint == endpoint) {
            return slot.playerId;
        }
    }
    return 255;
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
            break;

        case PacketType::PING:
            std::cout << "[PacketId=" << packetId << "]";
            {
                auto response = serializePingResponse(packetId, timestamp);
                _socket.send_to(asio::buffer(response), clientEndpoint);
            }
            break;

        case PacketType::SNAPSHOT:
            if (!payload.empty())
                std::cout << "[NbEntities=" << int(payload[0])
                        << "][EntityData=" << std::string(payload.begin()+1, payload.end()) << "]";
            break;

        case PacketType::ENTITY_EVENT:
            if (payload.size() >= 2)
                std::cout << "[EntityId=" << int(payload[0])
                        << "][EventType=" << int(payload[1])
                        << "][ExtraData=" << (payload.size() > 2 ? std::string(payload.begin()+2, payload.end()) : "") << "]";
            break;

        case PacketType::PLAYER_EVENT:
            if (payload.size() >= 2)
                std::cout << "[PlayerId=" << int(payload[0])
                        << "][EventType=" << int(payload[1])
                        << "][Score=" << (payload.size() > 2 ? int(payload[2]) : 0) << "]";
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

void rtype::NetworkServer::handleJoinPacket(
    const asio::ip::udp::endpoint& clientEndpoint,
    const std::vector<uint8_t>& payload)
{
    std::string username(payload.begin(), payload.end());
    std::cout << "[Username=" << username << "]";

    std::lock_guard<std::mutex> lock(_clientsMutex);

    for (const auto& slot : _playerSlots) {
        if (slot.isUsed && slot.endpoint == clientEndpoint) {
            std::cout << " [Already connected as Player " << int(slot.playerId) << "]" << std::endl;
            sendPlayerIdAssignment(clientEndpoint, slot.playerId);
            return;
        }
    }

    uint8_t assignedPlayerId = 255;
    for (auto& slot : _playerSlots) {
        if (!slot.isUsed) {
            slot.isUsed = true;
            slot.endpoint = clientEndpoint;
            slot.username = username;
            assignedPlayerId = slot.playerId;
            break;
        }
    }

    int clientId = _nextClientId++;
    _clients[clientId] = clientEndpoint;

    std::cout << " -> Assigned Player ID " << int(assignedPlayerId) 
              << ", Total players: " << countActivePlayers() << "/4" << std::endl;

    sendPlayerIdAssignment(clientEndpoint, assignedPlayerId);

    for (const auto& slot : _playerSlots) {
        if (slot.isUsed && slot.playerId != assignedPlayerId) {
            sendPlayerJoinEvent(clientEndpoint, slot.playerId);
        }
    }

    for (const auto& slot : _playerSlots) {
        if (slot.isUsed && slot.playerId != assignedPlayerId) {
            sendPlayerJoinEvent(slot.endpoint, assignedPlayerId);
        }
    }
}

void rtype::NetworkServer::sendPlayerIdAssignment(
    const asio::ip::udp::endpoint& clientEndpoint, 
    uint8_t playerId)
{
    std::vector<uint8_t> packet;
    packet.push_back(static_cast<uint8_t>(PacketType::PLAYER_ID_ASSIGNMENT));
    
    auto idBytes = toBytes<uint16_t>(0);
    packet.insert(packet.end(), idBytes.begin(), idBytes.end());
    
    auto tsBytes = toBytes<uint32_t>(0);
    packet.insert(packet.end(), tsBytes.begin(), tsBytes.end());
    
    packet.push_back(playerId);
    
    _socket.send_to(asio::buffer(packet), clientEndpoint);
    
    std::cout << "[SERVER] Sent PLAYER_ID_ASSIGNMENT(" << int(playerId) 
              << ") to " << clientEndpoint << std::endl;
}

void rtype::NetworkServer::sendPlayerJoinEvent(
    const asio::ip::udp::endpoint& clientEndpoint,
    uint8_t playerId)
{
    std::vector<uint8_t> packet;
    packet.push_back(static_cast<uint8_t>(PacketType::PLAYER_EVENT));
    
    auto idBytes = toBytes<uint16_t>(0);
    packet.insert(packet.end(), idBytes.begin(), idBytes.end());
    
    auto tsBytes = toBytes<uint32_t>(0);
    packet.insert(packet.end(), tsBytes.begin(), tsBytes.end());
    
    packet.push_back(playerId);
    packet.push_back(0); // 0 = JOIN event
    
    _socket.send_to(asio::buffer(packet), clientEndpoint);
    
    std::cout << "[SERVER] Sent PLAYER_JOIN(" << int(playerId) 
              << ") to " << clientEndpoint << std::endl;
}

int rtype::NetworkServer::countActivePlayers() const {
    int count = 0;
    for (const auto& slot : _playerSlots) {
        if (slot.isUsed) count++;
    }
    return count;
}

std::vector<uint8_t> rtype::NetworkServer::serializePingResponse(uint16_t packetId, uint32_t timestamp) {
    std::vector<uint8_t> packet;
    packet.push_back(static_cast<uint8_t>(PacketType::PING_RESPONSE));
    auto idBytes = toBytes(packetId);
    packet.insert(packet.end(), idBytes.begin(), idBytes.end());
    auto tsBytes = toBytes(timestamp);
    packet.insert(packet.end(), tsBytes.begin(), tsBytes.end());
    return packet;
}

void rtype::NetworkServer::broadcast(const std::vector<uint8_t>& message) {
    std::lock_guard<std::mutex> lock(_clientsMutex);
    for (auto& [id, endpoint] : _clients) {
        _socket.send_to(asio::buffer(message), endpoint);
    }
}
