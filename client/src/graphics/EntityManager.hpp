#pragma once

#include <unordered_map>
#include <vector>
#include <optional>
#include <memory>
#include <SFML/Graphics.hpp>
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
    std::optional<sf::Sprite> sprite;
    sf::Vector2f position;
    sf::Vector2f velocity;
    bool active;
    float scale;
    
    float scrollSpeed;
    bool looping;
    sf::FloatRect resetBounds;

    GameEntity();
};

class EntityManager {
private:
    std::unordered_map<uint32_t, GameEntity> _entities;
    std::unordered_map<RenderLayer, std::vector<uint32_t>> _layerMap;
    uint32_t _nextLocalId;

public:
    EntityManager();

    uint32_t createLocalEntity(EntityType type, RenderLayer layer);
    void createServerEntity(uint32_t serverId, EntityType type, RenderLayer layer);

    GameEntity* getEntity(uint32_t id);
    void removeEntity(uint32_t id);

    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    std::vector<GameEntity*> getEntitiesByType(EntityType type);

    void clear();
    size_t getEntityCount() const;
};

} // namespace CLIENT
