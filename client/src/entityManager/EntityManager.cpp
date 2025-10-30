/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** EntityManager.cpp
*/

#include "EntityManager.hpp"

#include <algorithm>
#include <iostream>

/**
 * @brief Default constructor for GameEntity.
 *
 * Initializes the entity with default values.
 */
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

/**
 * @brief Constructor for EntityManager.
 *
 * Initializes the local ID counter starting at 10000.
 */
CLIENT::EntityManager::EntityManager() : _nextLocalId(10000) {}

/**
 * @brief Creates a local entity with a unique ID.
 *
 * @return The unique ID of the newly created local entity.
 *
 * @details
 * The entity is marked as active and not part of the parallax system.
 */
uint32_t CLIENT::EntityManager::createLocalEntity()
{
    uint32_t id = _nextLocalId++;
    GameEntity& entity = _entities[id];
    entity.entityId = id;
    entity.active = true;
    entity.isParallax = false;

    return id;
}

/**
 * @brief Creates a parallax entity with a unique ID.
 *
 * @return The unique ID of the newly created parallax entity.
 *
 * @details
 * The entity is marked as active and is flagged as part of the parallax system.
 */
uint32_t CLIENT::EntityManager::createParallaxEntity()
{
    uint32_t id = _nextLocalId++;
    GameEntity& entity = _entities[id];
    entity.entityId = id;
    entity.active = true;
    entity.isParallax = true;

    return id;
}

/**
 * @brief Creates a simple entity associated with a server ID.
 *
 * @param serverId The ID assigned by the server.
 *
 * @details
 * If the entity already exists, it is reused and activated. Otherwise, a new
 * entity is created with default values and marked as active.
 */
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

/**
 * @brief Retrieves an entity by its ID.
 *
 * @param id The ID of the entity to retrieve.
 * @return Pointer to the GameEntity if found, nullptr otherwise.
 */
CLIENT::GameEntity* CLIENT::EntityManager::getEntity(uint32_t id)
{
    auto it = _entities.find(id);
    return (it != _entities.end()) ? &it->second : nullptr;
}

/**
 * @brief Removes an entity from the manager by its ID.
 *
 * @param id The ID of the entity to remove.
 *
 * @details
 * If the entity exists in the internal map, it is erased.
 */
void CLIENT::EntityManager::removeEntity(uint32_t id)
{
    auto it = _entities.find(id);
    if (it != _entities.end()) {
        _entities.erase(it);
    }
}

/**
 * @brief Deactivates all entities that are not in the given set of active IDs.
 *
 * @param activeIds Set of IDs representing currently active entities.
 *
 * @details
 * Only non-parallax entities with IDs less than 10000 are affected. Deactivated
 * entities have their sprite reset and currentSpritePath cleared.
 */
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

/**
 * @brief Removes all inactive, non-parallax entities with IDs < 10000.
 *
 * @details
 * Entities marked as inactive and not part of the parallax system are scheduled
 * for removal and then erased from the internal entity map.
 */
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

/**
 * @brief Updates all active entities with position and interpolation logic.
 *
 * @param deltaTime Time elapsed since the last update (in seconds).
 *
 * @details
 * Updates positions based on velocity, applies linear interpolation to target
 * positions, and handles looping entities within the window boundaries.
 */
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

/**
 * @brief Renders all active entities onto the given target.
 *
 * @param target The SFML render target to draw the entities on.
 *
 * @details
 * Parallax entities are drawn first, followed by non-parallax entities, to
 * ensure proper layering.
 */
void CLIENT::EntityManager::render(sf::RenderTarget& target)
{
    for (auto& [id, entity] : _entities) {
        if (entity.active && entity.sprite.has_value() && entity.isParallax) {
            target.draw(*entity.sprite);
        }
    }

    for (auto& [id, entity] : _entities) {
        if (entity.active && entity.sprite.has_value() && !entity.isParallax) {
            target.draw(*entity.sprite);
        }
    }
}

/**
 * @brief Clears all entities from the manager.
 *
 * @details
 * Erases all entities in the internal map, effectively resetting the manager.
 */
void CLIENT::EntityManager::clear()
{
    _entities.clear();
}

/**
 * @brief Returns the total number of entities managed.
 *
 * @return Total number of entities in the manager.
 */
size_t CLIENT::EntityManager::getEntityCount() const
{
    return _entities.size();
}

/**
 * @brief Returns the number of active entities.
 *
 * @return Number of entities marked as active.
 */
size_t CLIENT::EntityManager::getActiveEntityCount() const
{
    size_t count = 0;
    for (const auto& [id, entity] : _entities) {
        if (entity.active)
            count++;
    }
    return count;
}

/**
 * @brief Retrieves all active entities.
 *
 * @return A vector of pointers to all active GameEntity instances.
 */
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

/**
 * @brief Retrieves all active parallax entities.
 *
 * @return A vector of pointers to active GameEntity instances that are part
 *         of the parallax system.
 */
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
