/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** EntityManager.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "../macros.hpp"

namespace CLIENT {

struct GameEntity
{
    uint32_t entityId;
    bool active;
    bool isParallax;

    sf::Vector2f position;
    sf::Vector2f targetPosition;
    sf::Vector2f velocity;
    std::optional<sf::Sprite> sprite;

    float scale;
    float scrollSpeed;
    bool looping;

    std::string currentSpritePath;

    float interpolationTime;
    float interpolationDuration;

    GameEntity();
};

class EntityManager
{
   public:
    EntityManager();

    uint32_t createLocalEntity();

    uint32_t createParallaxEntity();

    void createSimpleEntity(uint32_t serverId);

    GameEntity* getEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void deactivateEntitiesNotInSet(const std::set<uint8_t>& activeIds);

    void cleanupInactiveEntities();

    void update(float deltaTime);

    void render(sf::RenderTarget &target);

    void clear();
    [[nodiscard]] size_t getEntityCount() const;
    [[nodiscard]] size_t getActiveEntityCount() const;

    std::vector<GameEntity*> getAllActiveEntities();

    std::vector<GameEntity*> getParallaxEntities();

   private:
    std::map<uint32_t, GameEntity> _entities;
    uint32_t _nextLocalId;
};

}  // namespace CLIENT
