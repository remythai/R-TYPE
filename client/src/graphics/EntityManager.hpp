/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** EntityManager.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <vector>
#include <optional>
#include <set>
#include <string>
#include "../macros.hpp"

namespace CLIENT {

    enum class RenderLayer {
        BACKGROUND = 0,
        PARALLAX_FAR = 1,
        PARALLAX_NEAR = 2,
        OBSTACLES = 3,
        ENEMIES = 4,
        PLAYERS = 5,
        PROJECTILES = 6,
        UI = 7
    };

    enum class EntityType {
        PLAYER,
        ENEMY,
        OBSTACLE,
        PROJECTILE,
        STAR,
        BACKGROUND,
        DECORATION
    };

    struct GameEntity {
        uint32_t entityId;
        EntityType type;
        RenderLayer layer;
        
        sf::Vector2f position;
        sf::Vector2f targetPosition;
        sf::Vector2f velocity;
        std::optional<sf::Sprite> sprite;
        
        bool active;
        float scale;
        float scrollSpeed;
        bool looping;
        
        std::string currentSpritePath;
        
        float interpolationTime;
        float interpolationDuration;

        GameEntity();
    };

    class EntityManager {
    public:
        EntityManager();

        uint32_t createLocalEntity(EntityType type, RenderLayer layer);
        void createServerEntity(uint32_t serverId, EntityType type, RenderLayer layer);
        
        GameEntity* getEntity(uint32_t id);
        std::vector<GameEntity*> getEntitiesByType(EntityType type);
        
        void removeEntity(uint32_t id);
        void deactivateEntitiesNotInSet(const std::set<uint8_t>& activeIds);
        void cleanupInactiveEntities();
        
        void update(float deltaTime);
        void render(sf::RenderWindow& window);
        
        void clear();
        size_t getEntityCount() const;
        size_t getActiveEntityCount() const;

    private:
        std::map<uint32_t, GameEntity> _entities;
        std::map<RenderLayer, std::vector<uint32_t>> _layerMap;
        uint32_t _nextLocalId;
    };

} // namespace CLIENT