/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** ParallaxSystem.cpp
*/

#include "ParallaxSystem.hpp"
#include <iostream>

CLIENT::ParallaxSystem::ParallaxSystem(EntityManager* entityMgr, ResourceManager* resourceMgr)
    : _entityManager(entityMgr), _resourceManager(resourceMgr)
{
}

void CLIENT::ParallaxSystem::addLayer(const std::string& texturePath, float speed, float depth)
{
    _layers.emplace_back(texturePath, speed, depth);
    std::cout << "[ParallaxSystem] Added layer: " << texturePath 
              << " (speed=" << speed << ", depth=" << depth << ")\n";
}

void CLIENT::ParallaxSystem::createLayers()
{
    std::cout << "[ParallaxSystem] Creating " << _layers.size() << " layers...\n";
    
    for (const auto& layer : _layers) {
        createLayerEntities(layer);
    }
    
    std::cout << "[ParallaxSystem] All layers created\n";
}

void CLIENT::ParallaxSystem::createLayerEntities(const ParallaxLayer& layer)
{
    auto* texture = _resourceManager->getTexture(layer.texturePath);
    if (!texture) {
        std::cerr << "[ParallaxSystem] Failed to load texture: " << layer.texturePath << "\n";
        return;
    }

    float scaleY = static_cast<float>(WINDOW_HEIGHT) / texture->getSize().y;
    float scaleX = scaleY;
    float textureWidth = texture->getSize().x * scaleX;

    RenderLayer renderLayer = getRenderLayerFromDepth(layer.depth);

    for (int i = 0; i < 3; ++i) {
        uint32_t id = _entityManager->createLocalEntity(EntityType::BACKGROUND, renderLayer);
        auto* entity = _entityManager->getEntity(id);
        
        if (entity) {
            entity->sprite = sf::Sprite(*texture);
            entity->position = sf::Vector2f(static_cast<float>(i) * textureWidth, 0.0f);
            entity->velocity = sf::Vector2f(-layer.speed, 0.0f);
            entity->looping = false;
            entity->scrollSpeed = textureWidth;

            entity->sprite->setPosition(entity->position);
            entity->sprite->setScale(sf::Vector2f(scaleX, scaleY));

            std::cout << "[ParallaxSystem] Created entity for " << layer.texturePath
                      << " at x=" << entity->position.x
                      << " (width=" << textureWidth
                      << ", layer=" << static_cast<int>(renderLayer) << ")\n";
        }
    }
}

CLIENT::RenderLayer CLIENT::ParallaxSystem::getRenderLayerFromDepth(float depth)
{
    if (depth < 0.5f) {
        return RenderLayer::PARALLAX_FAR;
    } else {
        return RenderLayer::PARALLAX_NEAR;
    }
}

void CLIENT::ParallaxSystem::update(float deltaTime)
{
    for (int layer = static_cast<int>(RenderLayer::PARALLAX_FAR); 
         layer <= static_cast<int>(RenderLayer::PARALLAX_NEAR); 
         ++layer) {
        
        RenderLayer currentLayer = static_cast<RenderLayer>(layer);
        auto layerEntities = _entityManager->getEntitiesByLayer(currentLayer);
        
        for (auto* entity : layerEntities) {
            if (!entity || !entity->sprite.has_value()) continue;
            
            entity->position.x += entity->velocity.x * deltaTime;
            entity->position.y += entity->velocity.y * deltaTime;
            
            if (entity->looping && entity->scrollSpeed > 0) {
                if (entity->position.x + entity->scrollSpeed < 0) {
                    entity->position.x += entity->scrollSpeed * 3.0f;
                }
            }
            
            entity->sprite->setPosition(entity->position);
        }
    }
}

void CLIENT::ParallaxSystem::clear()
{
    _layers.clear();
    std::cout << "[ParallaxSystem] All layers cleared\n";
}