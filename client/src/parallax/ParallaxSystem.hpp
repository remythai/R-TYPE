/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** ParallaxSystem.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "../entityManager/EntityManager.hpp"
#include "../graphics/ResourceManager.hpp"

namespace CLIENT {

struct ParallaxLayer
{
    std::string texturePath;
    float scrollSpeed;
    float depth;
    std::vector<uint32_t> entityIds;
};

class ParallaxSystem
{
   public:
    ParallaxSystem(
        EntityManager* entityManager, ResourceManager* resourceManager);
    ~ParallaxSystem() = default;

    void addLayer(
        const std::string& texturePath, float scrollSpeed, float depth);
    void createLayers();
    void update(float deltaTime);
    void clear();

   private:
    void createTilesForLayer(ParallaxLayer& layer);

    EntityManager* _entityManager;
    ResourceManager* _resourceManager;
    std::vector<ParallaxLayer> _layers;

    static constexpr int TILES_PER_LAYER = 3;
};

}  // namespace CLIENT