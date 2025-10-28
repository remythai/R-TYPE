/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkClient.cpp
*/

#include "NetworkClient.hpp"

#include <chrono>
#include <cstring>
#include <iomanip>
#include <thread>

template <typename T>
std::vector<uint8_t> toBytes(T value)
{
    std::vector<uint8_t> bytes(sizeof(T));
    for (size_t i = 0; i < sizeof(T); ++i)
        bytes[sizeof(T) - 1 - i] = (value >> (i * 8)) & 0xFF;
    return bytes;
}

NetworkClient::NetworkClient(const std::string& host, unsigned short port)
    : _socket(_ioContext, asio::ip::udp::v4()), _recvBuffer(65535)
{
    asio::ip::udp::resolver resolver(_ioContext);
    auto endpoints =
        resolver.resolve(asio::ip::udp::v4(), host, std::to_string(port));
    _serverEndpoint = *endpoints.begin();
}

void NetworkClient::sendPacket(
    rtype::PacketType type, uint16_t packetId, uint32_t timestamp,
    const std::vector<uint8_t>& payload)
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
    std::memcpy(
        payload.data(), username.c_str(),
        std::min(size_t(32), username.size()));
    sendPacket(rtype::PacketType::JOIN, 0, 0, payload);
}

void NetworkClient::startReceiving()
{
    doReceive();
    std::thread([this]() { _ioContext.run(); }).detach();
}

void NetworkClient::setOnPlayerIdReceived(std::function<void(uint8_t)> callback)
{
    _onPlayerIdReceived = callback;
}

void NetworkClient::setOnPlayerEvent(
    std::function<void(uint8_t, uint8_t)> callback)
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
            if (!ec)
                doReceive();
        });
}

void NetworkClient::handlePacket(
    const std::vector<uint8_t>& buffer, size_t bytesReceived,
    const asio::ip::udp::endpoint& sender)
{
    if (bytesReceived < 7)
        return;

    rtype::PacketType type = static_cast<rtype::PacketType>(buffer[0]);
    uint16_t packetId = (buffer[1] << 8) | buffer[2];
    uint32_t timestamp =
        (buffer[3] << 24) | (buffer[4] << 16) | (buffer[5] << 8) | buffer[6];

    std::vector<uint8_t> payload(
        buffer.begin() + 7, buffer.begin() + bytesReceived);

    switch (type) {
        case rtype::PacketType::PLAYER_ID_ASSIGNMENT:
            if (!payload.empty()) {
                uint8_t playerId = payload[0];
                std::cout << "[CLIENT] ✓ PLAYER_ID_ASSIGNMENT received: "
                          << int(playerId) << std::endl;

                if (_onPlayerIdReceived) {
                    std::cout
                        << "[CLIENT] Calling _onPlayerIdReceived callback..."
                        << std::endl;
                    _onPlayerIdReceived(playerId);
                } else {
                    std::cout
                        << "[CLIENT] No callback set for _onPlayerIdReceived"
                        << std::endl;
                }
            }
            break;

        case rtype::PacketType::SNAPSHOT:
            if (!payload.empty()) {
                size_t offset = 0;

                while (offset < payload.size()) {
                    if (offset + 1 > payload.size())
                        break;
                    uint8_t entityId = payload[offset++];

                    auto readFloat = [&payload, &offset]() -> float {
                        if (offset + 4 > payload.size())
                            return 0.0f;
                        uint32_t temp =
                            (static_cast<uint32_t>(payload[offset]) << 24) |
                            (static_cast<uint32_t>(payload[offset + 1]) << 16) |
                            (static_cast<uint32_t>(payload[offset + 2]) << 8) |
                            static_cast<uint32_t>(payload[offset + 3]);
                        offset += 4;
                        float result;
                        std::memcpy(&result, &temp, sizeof(float));
                        return result;
                    };

                    if (offset + 8 > payload.size())
                        break;
                    float x = readFloat();
                    float y = readFloat();

                    if (offset >= payload.size())
                        break;
                    uint8_t pathLen = payload[offset++];

                    if (offset + pathLen > payload.size())
                        break;
                    std::string spritePath(
                        payload.begin() + offset,
                        payload.begin() + offset + pathLen);
                    offset += pathLen;

                    if (offset + 16 > payload.size())
                        break;
                    float rectPosX = readFloat();
                    float rectPosY = readFloat();
                    float rectSizeX = readFloat();
                    float rectSizeY = readFloat();

                    std::cout << "[Entity:" << int(entityId) << " pos:("
                              << std::fixed << std::setprecision(1) << x << ","
                              << y << ")" << " sprite:" << spritePath
                              << " rectPos:(" << rectPosX << "," << rectPosY
                              << ")" << " rectSize:(" << rectSizeX << ","
                              << rectSizeY << ")]" << std::endl;
                }

                if (_onSnapshot) {
                    _onSnapshot(payload);
                }
            }
            break;

        case rtype::PacketType::TIMEOUT:
            if (payload.size() >= 3) {
                size_t offset = 0;
                uint8_t entityId = payload[offset++];
                uint8_t playerId = payload[offset++];
                uint8_t usernameLen = payload[offset++];

                if (offset + usernameLen <= payload.size()) {
                    std::string username(
                        payload.begin() + offset,
                        payload.begin() + offset + usernameLen);

                    std::cout << "[CLIENT] TIMEOUT - Player " << int(playerId)
                              << " (" << username
                              << ") timed out. Entity: " << int(entityId)
                              << std::endl;

                    if (_onTimeout) {
                        _onTimeout(playerId);
                    }
                }
            }
            break;

        case rtype::PacketType::KILLED:
            if (payload.size() >= 3) {
                size_t offset = 0;
                uint8_t entityId = payload[offset++];
                uint8_t playerId = payload[offset++];
                uint8_t usernameLen = payload[offset++];

                if (offset + usernameLen <= payload.size()) {
                    std::string username(
                        payload.begin() + offset,
                        payload.begin() + offset + usernameLen);
                    std::cout << "[CLIENT] KILLED - Player " << int(playerId)
                              << " (" << username
                              << ") was eliminated. Entity: " << int(entityId)
                              << std::endl;
                    if (_onKilled) {
                        _onKilled(playerId);
                    }
                }
            }
            break;

        default:
            std::cout << "[CLIENT] Unhandled packet type: " << int(type)
                      << std::endl;
            break;
    }
}

void NetworkClient::setOnSnapshot(
    std::function<void(const std::vector<uint8_t>&)> callback)
{
    _onSnapshot = callback;
}

void NetworkClient::setOnTimeout(std::function<void(uint8_t)> callback)
{
    _onTimeout = callback;
}

void NetworkClient::setOnKilled(std::function<void(uint8_t)> callback)
{
    _onKilled = callback;
}
