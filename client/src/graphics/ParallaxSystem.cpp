/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** ParallaxSystem.cpp
*/

#include "ParallaxSystem.hpp"
#include <iostream>

CLIENT::ParallaxSystem::ParallaxSystem(EntityManager* entityManager, 
                                       ResourceManager* resourceManager)
    : _entityManager(entityManager), _resourceManager(resourceManager)
{
}

void CLIENT::ParallaxSystem::addLayer(const std::string& texturePath, 
                                       float scrollSpeed, float depth)
{
    ParallaxLayer layer;
    layer.texturePath = texturePath;
    layer.scrollSpeed = scrollSpeed;
    layer.depth = depth;
    _layers.push_back(layer);
}

void CLIENT::ParallaxSystem::createLayers()
{
    for (auto& layer : _layers) {
        createTilesForLayer(layer);
    }
}

void CLIENT::ParallaxSystem::createTilesForLayer(ParallaxLayer& layer)
{
    sf::Texture* texture = _resourceManager->getTexture(layer.texturePath);
    if (!texture) {
        std::cerr << "[ParallaxSystem] Failed to get texture: " << layer.texturePath << "\n";
        return;
    }

    sf::Vector2u textureSize = texture->getSize();
    float scaleY = static_cast<float>(WINDOW_HEIGHT) / textureSize.y;
    float scaleX = scaleY;
    float scaledWidth = textureSize.x * scaleX;

    for (int i = 0; i < TILES_PER_LAYER; ++i) {
        uint32_t entityId = _entityManager->createParallaxEntity();
        GameEntity* entity = _entityManager->getEntity(entityId);
        
        if (entity) {
            entity->sprite = sf::Sprite(*texture);
            entity->sprite->setScale(sf::Vector2f(scaleX, scaleY));
            entity->position = sf::Vector2f(i * scaledWidth, 0.0f);
            entity->sprite->setPosition(entity->position);
            entity->velocity = sf::Vector2f(-layer.scrollSpeed, 0.0f);
            entity->scrollSpeed = layer.scrollSpeed;
            entity->looping = true;
            entity->active = true;
            entity->isParallax = true;
            entity->currentSpritePath = layer.texturePath;
            
            layer.entityIds.push_back(entityId);
        }
    }
}

void CLIENT::ParallaxSystem::update(float deltaTime)
{
    for (auto& layer : _layers) {
        float rightmostX = -std::numeric_limits<float>::max();
        
        for (uint32_t entityId : layer.entityIds) {
            GameEntity* entity = _entityManager->getEntity(entityId);
            if (!entity || !entity->active || !entity->sprite.has_value()) continue;

            auto bounds = entity->sprite->getGlobalBounds();
            float entityRight = entity->position.x + bounds.size.x;
            
            if (entityRight > rightmostX) {
                rightmostX = entityRight;
            }
        }
        
        for (uint32_t entityId : layer.entityIds) {
            GameEntity* entity = _entityManager->getEntity(entityId);
            if (!entity || !entity->active || !entity->sprite.has_value()) continue;

            entity->position.x += entity->velocity.x * deltaTime;
            auto bounds = entity->sprite->getGlobalBounds();
            
            if (entity->position.x + bounds.size.x < 0) {
                entity->position.x = rightmostX;
            }
            
            entity->sprite->setPosition(entity->position);
        }
    }
}
void CLIENT::ParallaxSystem::clear()
{
    _layers.clear();
}
