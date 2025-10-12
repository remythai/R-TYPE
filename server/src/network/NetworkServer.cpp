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
std::vector<uint8_t> toBytes(T value)
{
    std::vector<uint8_t> bytes(sizeof(T));
    for (size_t i = 0; i < sizeof(T); ++i)
        bytes[sizeof(T) - 1 - i] = (value >> (i * 8)) & 0xFF;
    return bytes;
}

template<typename T>
T fromBytes(const uint8_t* data)
{
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i)
        value |= data[i] << (8 * (sizeof(T) - 1 - i));
    return value;
}

std::vector<uint8_t> floatToBytes(float value)
{
    std::vector<uint8_t> bytes(4);
    uint32_t temp;
    std::memcpy(&temp, &value, sizeof(float));
    bytes[0] = (temp >> 24) & 0xFF;
    bytes[1] = (temp >> 16) & 0xFF;
    bytes[2] = (temp >> 8) & 0xFF;
    bytes[3] = temp & 0xFF;
    return bytes;
}

rtype::NetworkServer::NetworkServer(unsigned short port, std::string const &hostname)
    : _socket(_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
    _running(false),
    _hostname(hostname),
    _registry(std::make_unique<Registry>())
{
    for (int i = 0; i < 4; ++i) {
        _playerSlots[i].isUsed = false;
        _playerSlots[i].playerId = i;
        _playerSlots[i].entity = EntityManager::INVALID_ENTITY;
    }
    
    initECS();
}

rtype::NetworkServer::~NetworkServer()
{
    stop();
}

void rtype::NetworkServer::initECS()
{
    _registry->addSystem<GameEngine::InputHandlerSystem>(0);
    _registry->addSystem<GameEngine::MotionSystem>(1);
    
    std::cout << "[SERVER] ECS initialized with InputHandlerSystem and MotionSystem" << std::endl;
}

EntityManager::Entity rtype::NetworkServer::createPlayerEntity(uint8_t playerId)
{
    auto entity = _registry->create();

    _registry->emplace<GameEngine::InputControlled>(entity);
    _registry->emplace<GameEngine::Acceleration>(entity, 0.0f, 0.0f);
    _registry->emplace<GameEngine::Position>(entity, 100.0f, 100.0f + playerId * 50.0f);
    _registry->emplace<GameEngine::Velocity>(entity, 1000.0f);
    _registry->emplace<GameEngine::Renderable>(entity, 1920.0f, 1080.0f);
    
    std::cout << "[SERVER] Created ECS entity " << entity << " for Player " << int(playerId) << std::endl;
    
    return entity;
}

void rtype::NetworkServer::destroyPlayerEntity(uint8_t playerId)
{
    if (playerId < 4 && _playerSlots[playerId].entity != EntityManager::INVALID_ENTITY) {
        _registry->destroy(_playerSlots[playerId].entity);
        _playerSlots[playerId].entity = EntityManager::INVALID_ENTITY;
        std::cout << "[SERVER] Destroyed ECS entity for Player " << int(playerId) << std::endl;
    }
}

void rtype::NetworkServer::applyInputToEntity(uint8_t playerId, uint8_t keyCode, uint8_t action)
{
    if (playerId >= 4 || !_playerSlots[playerId].isUsed) {
        return;
    }
    
    auto entity = _playerSlots[playerId].entity;
    if (entity == EntityManager::INVALID_ENTITY) {
        return;
    }
    
    if (!_registry->has<GameEngine::InputControlled>(entity)) {
        return;
    }
    
    auto& inputCtrl = _registry->get<GameEngine::InputControlled>(entity);

    if (action == 1) {
        if (std::find(inputCtrl.inputs.begin(), inputCtrl.inputs.end(), keyCode) == inputCtrl.inputs.end()) {
            inputCtrl.inputs.push_back(keyCode);
        }
    } else if (action == 0) {
        inputCtrl.inputs.erase(
            std::remove(inputCtrl.inputs.begin(), inputCtrl.inputs.end(), keyCode),
            inputCtrl.inputs.end()
        );
    }
}

void rtype::NetworkServer::updateECS(float dt)
{
    _registry->update(dt);
}

void rtype::NetworkServer::run()
{
    _running = true;
    _lastSnapshot = std::chrono::steady_clock::now();
    
    doReceive();
    std::cout << "UDP Server running..." << std::endl;

    std::thread([this]() {
        while (_running) {
            cleanInactivePlayers();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();

    std::thread([this]() {
        const float targetDt = 1.0f / 60.0f;
        auto lastUpdateTime = std::chrono::steady_clock::now();
        
        while (_running) {
            auto frameStart = std::chrono::steady_clock::now();

            float realDt = std::chrono::duration<float>(frameStart - lastUpdateTime).count();
            lastUpdateTime = frameStart;

            realDt = std::min(realDt, 0.25f);

            updateECS(realDt);

            auto frameEnd = std::chrono::steady_clock::now();
            float elapsed = std::chrono::duration<float>(frameEnd - frameStart).count();
            
            if (elapsed < targetDt) {
                std::this_thread::sleep_for(
                    std::chrono::duration<float>(targetDt - elapsed)
                );
            }
        }
    }).detach();
    
    std::thread([this]() {
        while (_running) {
            auto now = std::chrono::steady_clock::now();
            float elapsed = std::chrono::duration<float>(now - _lastSnapshot).count();
            
            if (elapsed >= SNAPSHOT_RATE) {
                broadcastSnapshot();
                _lastSnapshot = now;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }).detach();
    
    _ioContext.run();
}

void rtype::NetworkServer::cleanInactivePlayers()
{
    std::lock_guard<std::mutex> lock(_playerSlotsMutex);

    auto now = std::chrono::steady_clock::now();
    for (auto& slot : _playerSlots) {
        if (slot.isUsed) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - slot.lastActive).count();
            if (duration > 30) {
                std::cout << "[SERVER] Player " << int(slot.playerId)
                        << " (" << slot.username << ") timed out." << std::endl;

                std::vector<uint8_t> leaveEvent = {
                    static_cast<uint8_t>(rtype::PacketType::PLAYER_EVENT),
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    slot.playerId, 1
                };
                broadcast(leaveEvent);

                destroyPlayerEntity(slot.playerId);

                slot.isUsed = false;
            }
        }
    }
}

void rtype::NetworkServer::stop()
{
    _running = false;
    _ioContext.stop();
    
    std::lock_guard<std::mutex> lock(_clientsMutex);
    _clients.clear();

    for (auto& slot : _playerSlots) {
        if (slot.isUsed) {
            destroyPlayerEntity(slot.playerId);
        }
        slot.isUsed = false;
    }
    
    std::cout << "Server stopped." << std::endl;
}

void rtype::NetworkServer::doReceive()
{
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
    packet.push_back(0);

    _socket.send_to(asio::buffer(packet), clientEndpoint);

    std::cout << "[SERVER] Sent PLAYER_JOIN(" << int(playerId)
            << ") to " << clientEndpoint << std::endl;
}

int rtype::NetworkServer::countActivePlayers() const
{
    int count = 0;
    for (const auto& slot : _playerSlots) {
        if (slot.isUsed) count++;
    }
    return count;
}

std::vector<uint8_t> rtype::NetworkServer::serializePingResponse(uint16_t packetId, uint32_t timestamp)
{
    std::vector<uint8_t> packet;
    packet.push_back(static_cast<uint8_t>(PacketType::PING_RESPONSE));
    auto idBytes = toBytes(packetId);
    packet.insert(packet.end(), idBytes.begin(), idBytes.end());
    auto tsBytes = toBytes(timestamp);
    packet.insert(packet.end(), tsBytes.begin(), tsBytes.end());
    return packet;
}

void rtype::NetworkServer::broadcast(const std::vector<uint8_t>& message)
{
    std::lock_guard<std::mutex> lock(_clientsMutex);
    for (auto& [id, endpoint] : _clients) {
        _socket.send_to(asio::buffer(message), endpoint);
    }
}

std::vector<uint8_t> rtype::NetworkServer::serializeSnapshot()
{
    std::vector<uint8_t> snapshot;
    
    snapshot.push_back(static_cast<uint8_t>(PacketType::SNAPSHOT));
    
    auto idBytes = toBytes<uint16_t>(0);
    snapshot.insert(snapshot.end(), idBytes.begin(), idBytes.end());
    
    auto now = std::chrono::steady_clock::now();
    uint32_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    auto tsBytes = toBytes<uint32_t>(timestamp);
    snapshot.insert(snapshot.end(), tsBytes.begin(), tsBytes.end());
    
    uint8_t entityCount = 0;
    for (const auto& slot : _playerSlots) {
        if (slot.isUsed && slot.entity != EntityManager::INVALID_ENTITY) {
            entityCount++;
        }
    }
    snapshot.push_back(entityCount);
    
    for (const auto& slot : _playerSlots) {
        if (!slot.isUsed || slot.entity == EntityManager::INVALID_ENTITY) {
            continue;
        }
        
        auto entity = slot.entity;
        
        snapshot.push_back(slot.playerId);
        
        if (_registry->has<GameEngine::Position>(entity)) {
            auto& pos = _registry->get<GameEngine::Position>(entity);
            
            auto xBytes = floatToBytes(pos.x);
            snapshot.insert(snapshot.end(), xBytes.begin(), xBytes.end());
            
            auto yBytes = floatToBytes(pos.y);
            snapshot.insert(snapshot.end(), yBytes.begin(), yBytes.end());
        } else {
            auto xBytes = floatToBytes(0.0f);
            snapshot.insert(snapshot.end(), xBytes.begin(), xBytes.end());
            
            auto yBytes = floatToBytes(0.0f);
            snapshot.insert(snapshot.end(), yBytes.begin(), yBytes.end());
        }
        
        if (_registry->has<GameEngine::Velocity>(entity)) {
            auto& vel = _registry->get<GameEngine::Velocity>(entity);
            
            auto vxBytes = floatToBytes(vel.x);
            snapshot.insert(snapshot.end(), vxBytes.begin(), vxBytes.end());
            
            auto vyBytes = floatToBytes(vel.y);
            snapshot.insert(snapshot.end(), vyBytes.begin(), vyBytes.end());
        } else {
            auto vxBytes = floatToBytes(0.0f);
            snapshot.insert(snapshot.end(), vxBytes.begin(), vxBytes.end());
            
            auto vyBytes = floatToBytes(0.0f);
            snapshot.insert(snapshot.end(), vyBytes.begin(), vyBytes.end());
        }
        
        if (_registry->has<GameEngine::Acceleration>(entity)) {
            auto& acc = _registry->get<GameEngine::Acceleration>(entity);
            
            auto axBytes = floatToBytes(acc.x);
            snapshot.insert(snapshot.end(), axBytes.begin(), axBytes.end());
            
            auto ayBytes = floatToBytes(acc.y);
            snapshot.insert(snapshot.end(), ayBytes.begin(), ayBytes.end());
        } else {
            auto axBytes = floatToBytes(0.0f);
            snapshot.insert(snapshot.end(), axBytes.begin(), axBytes.end());
            
            auto ayBytes = floatToBytes(0.0f);
            snapshot.insert(snapshot.end(), ayBytes.begin(), ayBytes.end());
        }
    }
    
    return snapshot;
}

void rtype::NetworkServer::sendSnapshot(const asio::ip::udp::endpoint& clientEndpoint)
{
    auto snapshot = serializeSnapshot();
    _socket.send_to(asio::buffer(snapshot), clientEndpoint);
}

void rtype::NetworkServer::broadcastSnapshot()
{
    auto snapshot = serializeSnapshot();
    
    std::lock_guard<std::mutex> lock(_clientsMutex);
    for (auto& [id, endpoint] : _clients) {
        _socket.send_to(asio::buffer(snapshot), endpoint);
    }
}
