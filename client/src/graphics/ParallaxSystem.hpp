/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** ParallaxSystem.hpp
*/

#pragma once

#include "EntityManager.hpp"
#include "../graphics/ResourceManager.hpp"
#include <random>
#include <vector>

namespace CLIENT {

struct ParallaxLayer {
    std::string texturePath;
    float speed;
    float depth;
    
    ParallaxLayer(const std::string& path, float spd, float dpt)
        : texturePath(path), speed(spd), depth(dpt) {}
};

class ParallaxSystem {
public:
    ParallaxSystem(EntityManager* entityMgr, ResourceManager* resourceMgr);
    ~ParallaxSystem() = default;

    void addLayer(const std::string& texturePath, float speed, float depth);
    
    void createLayers();
    
    void update(float deltaTime);
    
    void clear();

private:
    EntityManager* _entityManager;
    ResourceManager* _resourceManager;
    std::vector<ParallaxLayer> _layers;
    
    void createLayerEntities(const ParallaxLayer& layer);
    
    RenderLayer getRenderLayerFromDepth(float depth);
};

} // namespace CLIENT