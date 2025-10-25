/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** EntityManager.cpp
*/

#include "EntityManager.hpp"

#include <algorithm>
#include <iostream>

CLIENT::GameEntity::GameEntity()
    : entityId(0),
      active(false),
      scale(1.0f),
      scrollSpeed(0.0f),
      looping(false),
      isParallax(false),
      currentSpritePath("")
{
}

CLIENT::EntityManager::EntityManager() : _nextLocalId(10000) {}

uint32_t CLIENT::EntityManager::createLocalEntity()
{
    uint32_t id = _nextLocalId++;
    GameEntity& entity = _entities[id];
    entity.entityId = id;
    entity.active = true;
    entity.isParallax = false;

    return id;
}

uint32_t CLIENT::EntityManager::createParallaxEntity()
{
    uint32_t id = _nextLocalId++;
    GameEntity& entity = _entities[id];
    entity.entityId = id;
    entity.active = true;
    entity.isParallax = true;

    return id;
}

void CLIENT::EntityManager::createSimpleEntity(uint32_t serverId)
{
    auto it = _entities.find(serverId);
    if (it != _entities.end()) {
        std::cout << "[EntityManager] Entity " << serverId
                  << " already exists, reusing\n";
        it->second.active = true;
        return;
    }

    GameEntity& entity = _entities[serverId];
    entity.entityId = serverId;
    entity.active = true;
    entity.isParallax = false;
}

CLIENT::GameEntity* CLIENT::EntityManager::getEntity(uint32_t id)
{
    auto it = _entities.find(id);
    return (it != _entities.end()) ? &it->second : nullptr;
}

void CLIENT::EntityManager::removeEntity(uint32_t id)
{
    auto it = _entities.find(id);
    if (it != _entities.end()) {
        _entities.erase(it);
    }
}

void CLIENT::EntityManager::deactivateEntitiesNotInSet(
    const std::set<uint8_t>& activeIds)
{
    for (auto& [id, entity] : _entities) {
        if (id >= 10000)
            continue;

        if (entity.isParallax)
            continue;

        if (activeIds.find(static_cast<uint8_t>(id)) == activeIds.end()) {
            if (entity.active) {
                std::cout << "[EntityManager] Deactivating entity " << id
                          << " (not in snapshot)\n";
                entity.active = false;
                entity.currentSpritePath = "";
                entity.sprite.reset();
            }
        }
    }
}

void CLIENT::EntityManager::cleanupInactiveEntities()
{
    std::vector<uint32_t> toRemove;

    for (auto& [id, entity] : _entities) {
        if (id < 10000 && !entity.active && !entity.isParallax) {
            toRemove.push_back(id);
        }
    }

    for (uint32_t id : toRemove) {
        removeEntity(id);
    }
}

void CLIENT::EntityManager::update(float deltaTime)
{
    for (auto& [id, entity] : _entities) {
        if (!entity.active || !entity.sprite.has_value())
            continue;

        if (entity.isParallax) {
            continue;
        }

        entity.position.x += entity.velocity.x * deltaTime;
        entity.position.y += entity.velocity.y * deltaTime;

        if (entity.interpolationDuration > 0.0f &&
            entity.interpolationTime < entity.interpolationDuration) {
            entity.interpolationTime += deltaTime;
            float alpha =
                entity.interpolationTime / entity.interpolationDuration;
            alpha = std::min(1.0f, std::max(0.0f, alpha));

            entity.position.x =
                entity.position.x +
                (entity.targetPosition.x - entity.position.x) * alpha;
            entity.position.y =
                entity.position.y +
                (entity.targetPosition.y - entity.position.y) * alpha;
        } else if (
            entity.interpolationDuration > 0.0f &&
            entity.interpolationTime >= entity.interpolationDuration) {
            entity.position = entity.targetPosition;
        }

        if (entity.looping) {
            auto bounds = entity.sprite->getGlobalBounds();

            if (entity.position.x + bounds.size.x < 0) {
                entity.position.x = WINDOW_WIDTH;
            }
            if (entity.position.y > WINDOW_HEIGHT) {
                entity.position.y = -bounds.size.y;
            }
        }

        entity.sprite->setPosition(entity.position);
    }
}

void CLIENT::EntityManager::render(sf::RenderWindow& window)
{
    for (auto& [id, entity] : _entities) {
        if (entity.active && entity.sprite.has_value() && entity.isParallax) {
            window.draw(*entity.sprite);
        }
    }

    for (auto& [id, entity] : _entities) {
        if (entity.active && entity.sprite.has_value() && !entity.isParallax) {
            window.draw(*entity.sprite);
        }
    }
}

void CLIENT::EntityManager::clear()
{
    _entities.clear();
}

size_t CLIENT::EntityManager::getEntityCount() const
{
    return _entities.size();
}

size_t CLIENT::EntityManager::getActiveEntityCount() const
{
    size_t count = 0;
    for (const auto& [id, entity] : _entities) {
        if (entity.active)
            count++;
    }
    return count;
}

std::vector<CLIENT::GameEntity*> CLIENT::EntityManager::getAllActiveEntities()
{
    std::vector<GameEntity*> result;
    for (auto& [id, entity] : _entities) {
        if (entity.active) {
            result.push_back(&entity);
        }
    }
    return result;
}

std::vector<CLIENT::GameEntity*> CLIENT::EntityManager::getParallaxEntities()
{
    std::vector<GameEntity*> result;
    for (auto& [id, entity] : _entities) {
        if (entity.active && entity.isParallax) {
            result.push_back(&entity);
        }
    }
    return result;
}
