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
      type(EntityType::DECORATION),
      layer(RenderLayer::BACKGROUND),
      active(false),
      scale(1.0f),
      scrollSpeed(0.0f),
      looping(false),
      currentSpritePath("") {}

CLIENT::EntityManager::EntityManager()
    : _nextLocalId(10000) {}

uint32_t CLIENT::EntityManager::createLocalEntity(EntityType type, RenderLayer layer) {
    uint32_t id = _nextLocalId++;
    GameEntity& entity = _entities[id];
    entity.entityId = id;
    entity.type = type;
    entity.layer = layer;
    entity.active = true;
    
    _layerMap[layer].push_back(id);
    return id;
}

void CLIENT::EntityManager::createServerEntity(uint32_t serverId, EntityType type, RenderLayer layer) {
    auto it = _entities.find(serverId);
    if (it != _entities.end()) {
        std::cout << "[EntityManager] Entity " << serverId << " already exists, reusing\n";
        it->second.active = true;
        return;
    }
    
    GameEntity& entity = _entities[serverId];
    entity.entityId = serverId;
    entity.type = type;
    entity.layer = layer;
    entity.active = true;

    _layerMap[layer].push_back(serverId);
}

CLIENT::GameEntity* CLIENT::EntityManager::getEntity(uint32_t id) {
    auto it = _entities.find(id);
    return (it != _entities.end()) ? &it->second : nullptr;
}

void CLIENT::EntityManager::removeEntity(uint32_t id) {
    auto it = _entities.find(id);
    if (it != _entities.end()) {
        RenderLayer layer = it->second.layer;
        _entities.erase(it);

        auto& layerVec = _layerMap[layer];
        layerVec.erase(std::remove(layerVec.begin(), layerVec.end(), id), layerVec.end());
    }
}

void CLIENT::EntityManager::deactivateEntitiesNotInSet(const std::set<uint8_t>& activeIds) {
    for (auto& [id, entity] : _entities) {
        if (id >= 10000) continue;
        
        if (activeIds.find(static_cast<uint8_t>(id)) == activeIds.end()) {
            if (entity.active) {
                std::cout << "[EntityManager] Deactivating entity " 
                          << id << " (not in snapshot)\n";
                entity.active = false;
                entity.currentSpritePath = "";
                entity.sprite.reset();
            }
        }
    }
}

void CLIENT::EntityManager::cleanupInactiveEntities() {
    std::vector<uint32_t> toRemove;
    
    for (auto& [id, entity] : _entities) {
        if (id < 10000 && !entity.active) {
            toRemove.push_back(id);
        }
    }
    
    for (uint32_t id : toRemove) {
        removeEntity(id);
    }
}

void CLIENT::EntityManager::update(float deltaTime) {
    for (auto& [id, entity] : _entities) {
        if (!entity.active || !entity.sprite.has_value()) continue;
        
        if (entity.layer == RenderLayer::PARALLAX_FAR || 
            entity.layer == RenderLayer::PARALLAX_NEAR) {
            continue;
        }
        
        entity.position.x += entity.velocity.x * deltaTime;
        entity.position.y += entity.velocity.y * deltaTime;
        
        if (entity.interpolationDuration > 0.0f && entity.interpolationTime < entity.interpolationDuration) {
            entity.interpolationTime += deltaTime;
            float alpha = entity.interpolationTime / entity.interpolationDuration;
            alpha = std::min(1.0f, std::max(0.0f, alpha));
            entity.position.x = entity.position.x + 
                (entity.targetPosition.x - entity.position.x) * alpha;
            entity.position.y = entity.position.y + 
                (entity.targetPosition.y - entity.position.y) * alpha;
        } else if (entity.interpolationDuration > 0.0f && entity.interpolationTime >= entity.interpolationDuration) {
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

void CLIENT::EntityManager::render(sf::RenderWindow& window) {
    for (int layer = 0; layer <= static_cast<int>(RenderLayer::UI); ++layer) {
        RenderLayer currentLayer = static_cast<RenderLayer>(layer);

        if (_layerMap.find(currentLayer) == _layerMap.end())
            continue;

        for (uint32_t entityId : _layerMap[currentLayer]) {
            auto* entity = getEntity(entityId);
            if (entity && entity->active && entity->sprite.has_value()) {
                window.draw(*entity->sprite);
            }
        }
    }
}

std::vector<CLIENT::GameEntity*> CLIENT::EntityManager::getEntitiesByType(EntityType type) {
    std::vector<GameEntity*> result;
    for (auto& [id, entity] : _entities) {
        if (entity.type == type && entity.active) {
            result.push_back(&entity);
        }
    }
    return result;
}

void CLIENT::EntityManager::clear() {
    _entities.clear();
    _layerMap.clear();
}

size_t CLIENT::EntityManager::getEntityCount() const {
    return _entities.size();
}

size_t CLIENT::EntityManager::getActiveEntityCount() const {
    size_t count = 0;
    for (const auto& [id, entity] : _entities) {
        if (entity.active) count++;
    }
    return count;
}

std::vector<CLIENT::GameEntity*> CLIENT::EntityManager::getEntitiesByLayer(RenderLayer layer) {
    std::vector<GameEntity*> result;
    if (_layerMap.find(layer) != _layerMap.end()) {
        for (uint32_t id : _layerMap[layer]) {
            auto* entity = getEntity(id);
            if (entity) {
                result.push_back(entity);
            }
        }
    }
    return result;
}
