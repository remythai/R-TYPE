/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkServer.cpp
*/

#include "NetworkServer.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

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

rtype::NetworkServer::NetworkServer(
    unsigned short port, std::string const& game)
    : _socket(_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
      _running(false),
      _game(game),
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
    _running = false;
    _ioContext.stop();

    std::lock_guard<std::mutex> lock(_clientsMutex);
    _clients.clear();

    for (auto& slot : _playerSlots) {
        if (slot.isUsed)
            destroyPlayerEntity(slot.playerId);
        slot.isUsed = false;
    }

    std::cout << "Server stopped.\n";
}

void rtype::NetworkServer::initECS()
{
    if (this->_game == std::string("flappyByte")) {
        std::cout << "[SERVER] ECS initialized with flappyByte Systems\n";
        _registry->addSystem<GameEngine::FPApplyGravity>(0);
        _registry->addSystem<GameEngine::FPInputHandler>(1);
        _registry->addSystem<GameEngine::FPMotion>(2);
    } else {
        std::cout << "[SERVER] ECS initialized with RTYPE Systems\n";
        _registry->addSystem<GameEngine::InputHandler>(0);
        _registry->addSystem<GameEngine::Motion>(1);
    }
    _registry->addSystem<GameEngine::Collision>(2);
    _registry->addSystem<GameEngine::Death>(3);
    _registry->addSystem<GameEngine::DomainHandler>(4);
    _registry->addSystem<GameEngine::Animation>(5);
}

void rtype::NetworkServer::updateECS(float dt)
{
    std::lock_guard<std::mutex> lock(_registryMutex);
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
        const float targetDt = 1.0f / 120.0f;
        auto lastUpdateTime = std::chrono::steady_clock::now();
        auto clock1 = std::chrono::steady_clock::now();
        auto clock2 = std::chrono::steady_clock::now();

        while (_running) {
            auto frameStart = std::chrono::steady_clock::now();

            float realDt =
                std::chrono::duration<float>(frameStart - lastUpdateTime)
                    .count();
            lastUpdateTime = frameStart;

            realDt = std::min(realDt, 0.25f);

            updateECS(realDt);

            auto frameEnd = std::chrono::steady_clock::now();
            float elapsed =
                std::chrono::duration<float>(frameEnd - frameStart).count();

            if (elapsed < targetDt) {
                std::this_thread::sleep_for(
                    std::chrono::duration<float>(targetDt - elapsed));
            }
            clock2 = std::chrono::steady_clock::now();
            auto deltaTime = clock2 - clock1;
            if (std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime)
                    .count() > 5000) {
                this->createEnemyEntity();
                clock1 = std::chrono::steady_clock::now();
            }
        }
    }).detach();

    std::thread([this]() {
        while (_running) {
            auto now = std::chrono::steady_clock::now();
            float elapsed =
                std::chrono::duration<float>(now - _lastSnapshot).count();

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
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                                now - slot.lastActive)
                                .count();
            if (duration > 30) {
                uint8_t playerId = slot.playerId;
                EntityManager::Entity entityId = slot.entity;
                std::string username = slot.username;

                {
                    std::lock_guard<std::mutex> regLock(_registryMutex);
                    if (entityId != EntityManager::INVALID_ENTITY) {
                        _registry->destroy(entityId);
                        slot.entity = EntityManager::INVALID_ENTITY;
                    }
                }

                slot.isUsed = false;

                std::vector<uint8_t> message;
                message.push_back(static_cast<uint8_t>(rtype::PacketType::TIMEOUT));

                auto idBytes = toBytes<uint16_t>(0);
                message.insert(message.end(), idBytes.begin(), idBytes.end());

                uint32_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                        now.time_since_epoch()).count();
                auto tsBytes = toBytes<uint32_t>(timestamp);
                message.insert(message.end(), tsBytes.begin(), tsBytes.end());

                message.push_back(static_cast<uint8_t>(entityId));
                message.push_back(playerId);

                uint8_t usernameLen = static_cast<uint8_t>(username.size());
                message.push_back(usernameLen);
                message.insert(message.end(), username.begin(), username.end());

                broadcast(message);

                std::cout << "[SERVER] Player " << int(playerId)
                        << " (" << username << ") timed out. Entity: "
                        << int(entityId) << std::endl;
            }
        }
    }
}

void rtype::NetworkServer::doReceive()
{
    auto buffer = std::make_shared<std::vector<uint8_t>>(1024);
    auto clientEndpoint = std::make_shared<asio::ip::udp::endpoint>();

    _socket.async_receive_from(
        asio::buffer(*buffer), *clientEndpoint,
        [this, buffer, clientEndpoint](
            std::error_code ec, std::size_t bytesReceived) {
            if (!ec && bytesReceived >= 7) {
                PacketType type = static_cast<PacketType>((*buffer)[0]);
                uint16_t packetId = fromBytes<uint16_t>(buffer->data() + 1);
                uint32_t timestamp = fromBytes<uint32_t>(buffer->data() + 3);

                size_t payloadSize = bytesReceived - 7;
                std::vector<uint8_t> payload(payloadSize);
                std::copy(
                    buffer->begin() + 7, buffer->begin() + 7 + payloadSize,
                    payload.begin());

                handleClientPacket(
                    *clientEndpoint, type, packetId, timestamp, payload);
            }
            if (_running)
                doReceive();
        });
}

std::string rtype::NetworkServer::packetTypeToString(rtype::PacketType type)
{
    switch (type) {
        case rtype::PacketType::INPUT:
            return "INPUT";
        case rtype::PacketType::JOIN:
            return "JOIN";
        case rtype::PacketType::SNAPSHOT:
            return "SNAPSHOT";
        case rtype::PacketType::PLAYER_ID_ASSIGNMENT:
            return "PLAYER_ID_ASSIGNMENT";
        case rtype::PacketType::TIMEOUT:
            return "TIMEOUT";
        case rtype::PacketType::KILLED:
            return "KILLED";
        default:
            return "UNKNOWN";
    }
}

void rtype::NetworkServer::sendPlayerIdAssignment(
    const asio::ip::udp::endpoint& clientEndpoint, uint8_t playerId)
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

int rtype::NetworkServer::countActivePlayers() const
{
    int count = 0;
    for (const auto& slot : _playerSlots) {
        if (slot.isUsed)
            count++;
    }
    return count;
}

void rtype::NetworkServer::broadcast(const std::vector<uint8_t>& message)
{
    std::lock_guard<std::mutex> lock(_clientsMutex);
    for (auto& [id, endpoint] : _clients)
        _socket.send_to(asio::buffer(message), endpoint);
}

std::vector<uint8_t> rtype::NetworkServer::serializeSnapshot()
{
    std::lock_guard<std::mutex> lock(_registryMutex);
    std::vector<uint8_t> snapshot;
    snapshot.push_back(static_cast<uint8_t>(PacketType::SNAPSHOT));

    auto idBytes = toBytes<uint16_t>(0);
    snapshot.insert(snapshot.end(), idBytes.begin(), idBytes.end());

    auto now = std::chrono::steady_clock::now();
    uint32_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch())
                            .count();
    auto tsBytes = toBytes<uint32_t>(timestamp);
    snapshot.insert(snapshot.end(), tsBytes.begin(), tsBytes.end());

    _registry->each<GameEngine::Renderable, GameEngine::Position>(
        [&](EntityManager::Entity entity, GameEngine::Renderable& render,
            GameEngine::Position& pos) {
            snapshot.push_back(static_cast<uint8_t>(entity));

            auto xBytes = floatToBytes(pos.pos.x);
            snapshot.insert(snapshot.end(), xBytes.begin(), xBytes.end());
            auto yBytes = floatToBytes(pos.pos.y);
            snapshot.insert(snapshot.end(), yBytes.begin(), yBytes.end());

            uint8_t pathLen =
                static_cast<uint8_t>(render.spriteSheetPath.size());
            snapshot.push_back(pathLen);
            snapshot.insert(
                snapshot.end(), render.spriteSheetPath.begin(),
                render.spriteSheetPath.end());

            auto rectPosX = floatToBytes(render.currentRectPos.x);
            snapshot.insert(snapshot.end(), rectPosX.begin(), rectPosX.end());
            auto rectPosY = floatToBytes(render.currentRectPos.y);
            snapshot.insert(snapshot.end(), rectPosY.begin(), rectPosY.end());

            auto rectSizeX = floatToBytes(render.rectSize.x);
            snapshot.insert(snapshot.end(), rectSizeX.begin(), rectSizeX.end());
            auto rectSizeY = floatToBytes(render.rectSize.y);
            snapshot.insert(snapshot.end(), rectSizeY.begin(), rectSizeY.end());
        });

    return snapshot;
}

void rtype::NetworkServer::broadcastSnapshot()
{
    auto snapshot = serializeSnapshot();

    std::lock_guard<std::mutex> lock(_clientsMutex);
    for (auto& [id, endpoint] : _clients)
        _socket.send_to(asio::buffer(snapshot), endpoint);
}
