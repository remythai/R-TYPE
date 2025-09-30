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
    : _socket(_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), _running(false), _hostname(hostname)
{}

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

                {
                    std::lock_guard<std::mutex> lock(_clientsMutex);
                    if (std::find_if(_clients.begin(), _clients.end(),
                                     [clientEndpoint](auto& p){ return p.second == *clientEndpoint; }) == _clients.end()) {
                        int clientId = _nextClientId++;
                        _clients[clientId] = *clientEndpoint;
                        std::cout << "Client " << clientId << " connected, Total clients: " << _clients.size() << std::endl;
                    }
                }

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
        default: return "UNKNOWN";
    }
}

void rtype::NetworkServer::handleClientPacket(
    const asio::ip::udp::endpoint& clientEndpoint,
    PacketType type, uint16_t packetId, uint32_t timestamp,
    const std::vector<uint8_t>& payload)
{
    std::cout << "[SERVER] From " << clientEndpoint << " -> ";

    std::cout << "[Type=" << std::hex << packetTypeToString(type) << "]";
    std::cout << "[PacketId=" << packetId << "]";
    std::cout << "[Timestamp=" << timestamp << "]";

    switch(type) {
        case PacketType::INPUT:
            if (payload.size() >= 3)
                std::cout << "[PlayerId=" << int(payload[0])
                        << "][KeyCode=" << int(payload[1])
                        << "][Action=" << int(payload[2]) << "]";
            break;

        case PacketType::JOIN:
            std::cout << "[Username=" << std::string(payload.begin(), payload.end()) << "]";
            break;

        case PacketType::PING:
            std::cout << "[PacketId=" << packetId << "]";
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

    if(type == PacketType::PING) {
        auto response = serializePingResponse(packetId, timestamp);
        _socket.send_to(asio::buffer(response), clientEndpoint);
    }
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
