/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** manageEntities.cpp
*/

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

#include "NetworkServer.hpp"

/**
 * @brief Creates a player entity in the ECS with appropriate components
 * 
 * Creates different player configurations based on the current game mode
 * (flappyByte or RType). Adds components for input control, position,
 * velocity, rendering, collision, health, and damage.
 * 
 * @param playerId The ID of the player (0-3)
 * @return EntityManager::Entity The created player entity
 */
EntityManager::Entity rtype::NetworkServer::createPlayerEntity(uint8_t playerId)
{
    std::lock_guard<std::mutex> lock(_registryMutex);
    auto entity = _registry->create();

    if (this->_game == std::string("flappyByte")) {
        std::vector<vec2> rectPos;
        rectPos.push_back(
            vec2{66.4f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{33.2f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{0.0f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{33.2f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{66.4f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{99.6f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{132.8f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{99.6f, static_cast<float>(int(17.2f * playerId) % 86)});
        _registry->emplace<GameEngine::InputControlled>(entity);
        _registry->emplace<GameEngine::Acceleration>(entity, 0.0f, 0.0f);
        _registry->emplace<GameEngine::Position>(
            entity, 100.0f, 100.0f + playerId * 50.0f);
        _registry->emplace<GameEngine::Velocity>(entity, 500.0f);
        _registry->emplace<GameEngine::Renderable>(
            entity, 1920.0f, 1080.0f, "assets/sprites/r-typesheet42.png",
            rectPos, vec2{33.2f, 17.2f}, 500, false);
        _registry->emplace<GameEngine::Collider>(
            entity, vec2(0.0, 0.0), std::bitset<8>("01000000"),
            std::bitset<8>("10000000"), vec2(33.2, 17.2));
        _registry->emplace<GameEngine::Health>(entity, 1, 1);
        _registry->emplace<GameEngine::Damage>(entity, 0);
        _registry->emplace<GameEngine::Gravity>(entity, 400.0f);
    } else {
        std::vector<vec2> rectPos;
        rectPos.push_back(
            vec2{66.4f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{33.2f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{0.0f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{33.2f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{66.4f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{99.6f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{132.8f, static_cast<float>(int(17.2f * playerId) % 86)});
        rectPos.push_back(
            vec2{99.6f, static_cast<float>(int(17.2f * playerId) % 86)});
        _registry->emplace<GameEngine::InputControlled>(entity);
        _registry->emplace<GameEngine::Acceleration>(entity, 0.0f, 0.0f);
        _registry->emplace<GameEngine::Position>(
            entity, 100.0f, 100.0f + playerId * 50.0f);
        _registry->emplace<GameEngine::Velocity>(entity, 5.0f);
        _registry->emplace<GameEngine::Renderable>(
            entity, 1920.0f, 1080.0f, "assets/sprites/r-typesheet42.png",
            rectPos, vec2{33.2f, 17.2f}, 500, false);
        _registry->emplace<GameEngine::Collider>(
            entity, vec2(0.0, 0.0), std::bitset<8>("01000000"),
            std::bitset<8>("10000000"), vec2(33.2, 17.2));
        _registry->emplace<GameEngine::Health>(entity, 1, 1);
        _registry->emplace<GameEngine::Damage>(entity, 1);
    }

    std::cout << "[SERVER] Created ECS entity " << entity << " for Player "
              << int(playerId) << std::endl;

    return entity;
}

/**
 * @brief Creates enemy entities with random positioning
 * 
 * Creates different enemy configurations based on the current game mode.
 * For flappyByte, creates obstacle patterns with gaps.
 * For RType, creates a single enemy at a random vertical position.
 * 
 * @return EntityManager::Entity Returns 1 after enemy creation
 */
EntityManager::Entity rtype::NetworkServer::createEnemyEntity()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(3, 27);

    int randomNum = distrib(gen);

    if (this->_game == std::string("flappyByte")) {
        for (size_t i = 0; i < randomNum; i++) {
            std::lock_guard<std::mutex> lock(_registryMutex);
            auto entity = _registry->create();
            _registry->emplace<GameEngine::AIControlled>(entity);
            _registry->emplace<GameEngine::Acceleration>(entity, -200.0f, 0.0f);
            _registry->emplace<GameEngine::Position>(entity, 1900, i * 34);
            _registry->emplace<GameEngine::Velocity>(entity, 200.0f);
            std::vector<vec2> rectPos;
            rectPos.push_back(vec2{0.0f, 0.0f});
            rectPos.push_back(vec2{34.0f, 0.0f});
            rectPos.push_back(vec2{68.0f, 0.0f});
            _registry->emplace<GameEngine::Renderable>(
                entity, 1920.0f, 1080.0f, "assets/sprites/r-typesheet30a.png",
                rectPos, vec2{34.0f, 34.0f}, 500, true);
            _registry->emplace<GameEngine::Collider>(
                entity, vec2(0.0, 0.0), std::bitset<8>("10000000"),
                std::bitset<8>("01000000"), vec2(34.0, 34.0));
            _registry->emplace<GameEngine::Domain>(
                entity, 5.0f, 0.0f, 1920.0f, 1080.0);
            _registry->emplace<GameEngine::Health>(entity, 1, 1);
            _registry->emplace<GameEngine::Damage>(entity, 1);
        }
        for (size_t i = 1; i < 29 - randomNum; i++) {
            std::lock_guard<std::mutex> lock(_registryMutex);
            auto entity = _registry->create();
            _registry->emplace<GameEngine::AIControlled>(entity);
            _registry->emplace<GameEngine::Acceleration>(entity, -200.0f, 0.0f);
            _registry->emplace<GameEngine::Position>(
                entity, 1900, 1080 - i * 34);
            _registry->emplace<GameEngine::Velocity>(entity, 200.0f);
            std::vector<vec2> rectPos;
            rectPos.push_back(vec2{0.0f, 0.0f});
            rectPos.push_back(vec2{34.0f, 0.0f});
            rectPos.push_back(vec2{68.0f, 0.0f});
            _registry->emplace<GameEngine::Renderable>(
                entity, 1920.0f, 1080.0f, "assets/sprites/r-typesheet30a.png",
                rectPos, vec2{34.0f, 34.0f}, 500, true);
            _registry->emplace<GameEngine::Collider>(
                entity, vec2(0.0, 0.0), std::bitset<8>("10000000"),
                std::bitset<8>("01000000"), vec2(34.0, 34.0));
            _registry->emplace<GameEngine::Domain>(
                entity, 5.0f, 0.0f, 1920.0f, 1080.0);
            _registry->emplace<GameEngine::Health>(entity, 1, 1);
            _registry->emplace<GameEngine::Damage>(entity, 1);
        }
    } else {
        std::lock_guard<std::mutex> lock(_registryMutex);
        auto entity = _registry->create();
        _registry->emplace<GameEngine::AIControlled>(entity);
        _registry->emplace<GameEngine::Acceleration>(entity, -3.0f, 0.0f);
        _registry->emplace<GameEngine::Position>(entity, 1900, randomNum);
        _registry->emplace<GameEngine::Velocity>(entity, 3.0f);
        std::vector<vec2> rectPos;
        rectPos.push_back(vec2{0.0f, 0.0f});
        rectPos.push_back(vec2{33.3f, 0.0f});
        rectPos.push_back(vec2{66.6f, 0.0f});
        rectPos.push_back(vec2{99.9f, 0.0f});
        rectPos.push_back(vec2{133.2f, 0.0f});
        rectPos.push_back(vec2{166.5f, 0.0f});
        rectPos.push_back(vec2{199.8f, 0.0f});
        rectPos.push_back(vec2{233.1f, 0.0f});
        _registry->emplace<GameEngine::Renderable>(
            entity, 1920.0f, 1080.0f, "assets/sprites/r-typesheet5.png",
            rectPos, vec2{33.3f, 36.0f}, 1000, true);
        _registry->emplace<GameEngine::Collider>(
            entity, vec2(0.0, 0.0), std::bitset<8>("10100000"),
            std::bitset<8>("01000000"), vec2(33.3, 33.3));
        _registry->emplace<GameEngine::Domain>(
            entity, 5.0f, 0.0f, 1920.0f, 1080.0);
        _registry->emplace<GameEngine::Health>(entity, 1, 1);
        _registry->emplace<GameEngine::Damage>(entity, 1);
    }

    return 1;
}

/**
 * @brief Destroys a player entity and broadcasts the death event
 * 
 * Removes the player entity from the ECS registry, clears the player slot,
 * and sends a KILLED packet to all connected clients.
 * 
 * @param playerId The ID of the player to destroy (0-3)
 */
void rtype::NetworkServer::destroyPlayerEntity(uint8_t playerId)
{
    std::lock_guard<std::mutex> lock(_registryMutex);
    if (playerId < 4 &&
        _playerSlots[playerId].entity != EntityManager::INVALID_ENTITY) {
        EntityManager::Entity entityId = _playerSlots[playerId].entity;
        std::string username = _playerSlots[playerId].username;

        _registry->destroy(_playerSlots[playerId].entity);
        _playerSlots[playerId].entity = EntityManager::INVALID_ENTITY;

        std::vector<uint8_t> message;
        message.push_back(static_cast<uint8_t>(rtype::PacketType::KILLED));

        auto idBytes = toBytes<uint16_t>(0);
        message.insert(message.end(), idBytes.begin(), idBytes.end());

        auto now = std::chrono::steady_clock::now();
        uint32_t timestamp =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch())
                .count();
        auto tsBytes = toBytes<uint32_t>(timestamp);
        message.insert(message.end(), tsBytes.begin(), tsBytes.end());

        message.push_back(static_cast<uint8_t>(entityId));

        message.push_back(playerId);

        uint8_t usernameLen = static_cast<uint8_t>(username.size());
        message.push_back(usernameLen);

        message.insert(message.end(), username.begin(), username.end());

        broadcast(message);
    }
}

/**
 * @brief Applies input actions to a player's entity
 * 
 * Updates the InputControlled component of the player's entity based on
 * the received key code and action (press/release).
 * 
 * @param playerId The ID of the player (0-3)
 * @param keyCode The key code of the input action
 * @param action The action type (1 for press, 0 for release)
 */
void rtype::NetworkServer::applyInputToEntity(
    uint8_t playerId, uint8_t keyCode, uint8_t action)
{
    if (playerId >= 4 || !_playerSlots[playerId].isUsed)
        return;

    std::lock_guard<std::mutex> lock(_registryMutex);

    auto entity = _playerSlots[playerId].entity;
    if (entity == EntityManager::INVALID_ENTITY)
        return;

    if (!_registry->has<GameEngine::InputControlled>(entity))
        return;

    auto& inputCtrl = _registry->get<GameEngine::InputControlled>(entity);

    if (action == 1) {
        if (std::find(
                inputCtrl.inputs.begin(), inputCtrl.inputs.end(), keyCode) ==
            inputCtrl.inputs.end()) {
            inputCtrl.inputs.push_back(keyCode);
        }
    } else if (action == 0) {
        inputCtrl.inputs.erase(
            std::remove(
                inputCtrl.inputs.begin(), inputCtrl.inputs.end(), keyCode),
            inputCtrl.inputs.end());
    }
}
