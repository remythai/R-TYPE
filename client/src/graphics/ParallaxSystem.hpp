#pragma once

#include "EntityManager.hpp"
#include "ResourceManager.hpp"
#include "../macros.hpp"
#include <random>

namespace CLIENT {

class ParallaxSystem {
private:
    EntityManager* _entityManager;
    ResourceManager* _resourceManager;
    std::mt19937 _rng;

public:
    ParallaxSystem(EntityManager* entityMgr, ResourceManager* resourceMgr);

    void createParallaxLayer(const std::string& texturePath, float speed, RenderLayer layer);

    void createParallaxLayers();

    void createStarField(int numStars, float minSpeed, float maxSpeed, RenderLayer layer);

    void createScrollingCloud(const std::string& texturePath, float speed, float yPos, RenderLayer layer);

    void createObstacle(uint32_t serverId, float x, float y, const std::string& texturePath, uint8_t size);
};

} // namespace CLIENT
