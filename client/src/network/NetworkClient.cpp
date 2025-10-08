/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkClient.cpp
*/

#include "NetworkClient.hpp"
#include <thread>
#include <chrono>
#include <cstring>

template<typename T>
std::vector<uint8_t> toBytes(T value)
{
    std::vector<uint8_t> bytes(sizeof(T));
    for (size_t i = 0; i < sizeof(T); ++i)
        bytes[sizeof(T) - 1 - i] = (value >> (i * 8)) & 0xFF;
    return bytes;
}

NetworkClient::NetworkClient(const std::string& host, unsigned short port)
    : _socket(_ioContext, asio::ip::udp::v4()),
    _recvBuffer(1024)
{
    asio::ip::udp::resolver resolver(_ioContext);
    auto endpoints = resolver.resolve(asio::ip::udp::v4(), host, std::to_string(port));
    _serverEndpoint = *endpoints.begin();
}

void NetworkClient::sendPacket(rtype::PacketType type, uint16_t packetId, uint32_t timestamp, const std::vector<uint8_t>& payload)
{
    std::vector<uint8_t> packet;
    packet.push_back(static_cast<uint8_t>(type));

    auto idBytes = toBytes(packetId);
    packet.insert(packet.end(), idBytes.begin(), idBytes.end());

    auto tsBytes = toBytes(timestamp);
    packet.insert(packet.end(), tsBytes.begin(), tsBytes.end());

    packet.insert(packet.end(), payload.begin(), payload.end());

    _socket.send_to(asio::buffer(packet), _serverEndpoint);
}

void NetworkClient::sendInput(uint8_t playerId, uint8_t keyCode, uint8_t action)
{
    sendPacket(rtype::PacketType::INPUT, 0, 0, {playerId, keyCode, action});
}

void NetworkClient::sendJoin(const std::string& username)
{
    std::vector<uint8_t> payload(32, 0);
    std::memcpy(payload.data(), username.c_str(), std::min(size_t(32), username.size()));
    sendPacket(rtype::PacketType::JOIN, 0, 0, payload);
}

void NetworkClient::sendPing(uint16_t packetId)
{
    sendPacket(rtype::PacketType::PING, packetId, 0, {});
}

void NetworkClient::startReceiving()
{
    doReceive();
    std::thread([this](){ _ioContext.run(); }).detach();
}

void NetworkClient::setOnPlayerIdReceived(std::function<void(uint8_t)> callback)
{
    _onPlayerIdReceived = callback;
}

void NetworkClient::setOnPlayerEvent(std::function<void(uint8_t, uint8_t)> callback)
{
    _onPlayerEvent = callback;
}

void NetworkClient::doReceive()
{
    auto sender = std::make_shared<asio::ip::udp::endpoint>();
    _socket.async_receive_from(
        asio::buffer(_recvBuffer), *sender,
        [this, sender](std::error_code ec, std::size_t bytesReceived) {
            if (!ec && bytesReceived > 0) {
                handlePacket(_recvBuffer, bytesReceived, *sender);
            }
            if (!ec) doReceive();
        }
    );
}

void NetworkClient::handlePacket(
    const std::vector<uint8_t>& buffer, size_t bytesReceived,
    const asio::ip::udp::endpoint& sender)
{
    if (bytesReceived < 7)
        return;

    rtype::PacketType type = static_cast<rtype::PacketType>(buffer[0]);
    uint16_t packetId = (buffer[1] << 8) | buffer[2];
    uint32_t timestamp = (buffer[3] << 24) | (buffer[4] << 16) | (buffer[5] << 8) | buffer[6];

    std::vector<uint8_t> payload(buffer.begin() + 7, buffer.begin() + bytesReceived);

    std::cout << "[CLIENT] From " << sender << " -> ";

    std::cout << "[Type=0x" << std::hex << int(type) << std::dec << "]";
    std::cout << "[PacketId=" << packetId << "]";
    std::cout << "[Timestamp=" << timestamp << "]";

    switch(type) {
        case rtype::PacketType::PLAYER_ID_ASSIGNMENT:
            if (!payload.empty()) {
                uint8_t playerId = payload[0];
                std::cout << "[AssignedPlayerId=" << int(playerId) << "]";

                if (playerId == 30) {
                    std::cout << "\n[Error] Server full, cannot join." << std::endl;
                    std::exit(1);
                }

                if (_onPlayerIdReceived) {
                    _onPlayerIdReceived(playerId);
                }
            }
            break;

        case rtype::PacketType::PLAYER_EVENT:
            if (payload.size() >= 2) {
                uint8_t playerId = payload[0];
                uint8_t eventType = payload[1];
                
                std::cout << "[PlayerId=" << int(playerId)
                        << "][EventType=" << int(eventType);
                
                if (payload.size() > 2)
                    std::cout << "][Score=" << int(payload[2]);
                std::cout << "]";
                
                if (_onPlayerEvent) {
                    _onPlayerEvent(playerId, eventType);
                }
            }
            break;

        case rtype::PacketType::INPUT:
            if (payload.size() >= 3)
                std::cout << "[PlayerId=" << int(payload[0])
                        << "][KeyCode=" << int(payload[1])
                        << "][Action=" << int(payload[2]) << "]";
            break;

        case rtype::PacketType::JOIN:
            std::cout << "[Username=" << std::string(payload.begin(), payload.end()) << "]";
            break;

        case rtype::PacketType::PING:
        case rtype::PacketType::PING_RESPONSE:
            std::cout << "[PacketId=" << packetId << "]";
            break;

        case rtype::PacketType::SNAPSHOT:
            if (!payload.empty())
                std::cout << "[NbEntities=" << int(payload[0])
                        << "][EntityData=" << std::string(payload.begin()+1, payload.end()) << "]";
            break;

        case rtype::PacketType::ENTITY_EVENT:
            if (payload.size() >= 2)
                std::cout << "[EntityId=" << int(payload[0])
                        << "][EventType=" << int(payload[1])
                        << "][ExtraData=" << (payload.size() > 2 ? std::string(payload.begin()+2, payload.end()) : "") << "]";
            break;

        default:
            std::cout << "[Unknown packet]";
            break;
    }

    std::cout << std::endl;
}