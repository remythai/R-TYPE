#include "ParallaxSystem.hpp"
#include <iostream>
#include <SFML/Graphics.hpp>

CLIENT::ParallaxSystem::ParallaxSystem(EntityManager* entityMgr, ResourceManager* resourceMgr)
    : _entityManager(entityMgr), _resourceManager(resourceMgr)
{
    std::random_device rd;
    _rng.seed(rd());
}

void CLIENT::ParallaxSystem::createParallaxLayer(const std::string& texturePath, float speed, RenderLayer layer)
{
    auto* texture = _resourceManager->getTexture(texturePath);
    if (!texture) {
        std::cerr << "Failed to load parallax texture: " << texturePath << "\n";
        return;
    }

    float scaleY = static_cast<float>(WINDOW_HEIGHT) / texture->getSize().y;
    float scaleX = scaleY;
    float textureWidth = texture->getSize().x * scaleX;

    for (int i = 0; i < 2; ++i) {
        uint32_t id = _entityManager->createLocalEntity(EntityType::BACKGROUND, layer);
        auto* entity = _entityManager->getEntity(id);
        if (entity) {
            entity->sprite = sf::Sprite(*texture);
            entity->position = sf::Vector2f(static_cast<float>(i * textureWidth), 0);
            entity->velocity = sf::Vector2f(-speed, 0);
            entity->looping = true;
            entity->scrollSpeed = textureWidth;

            entity->sprite->setPosition(entity->position);
            entity->sprite->setScale(sf::Vector2f(scaleX, scaleY));

            std::cout << "Created parallax layer " << texturePath
                      << " at x=" << entity->position.x
                      << " (width=" << textureWidth << ")\n";
        }
    }
}

void CLIENT::ParallaxSystem::createParallaxLayers()
{
    createParallaxLayer("assets/sprites/parallax/1.png", 10.0f, RenderLayer::PARALLAX_FAR);
    createParallaxLayer("assets/sprites/parallax/2.png", 25.0f, RenderLayer::PARALLAX_FAR);
    createParallaxLayer("assets/sprites/parallax/3.png", 50.0f, RenderLayer::PARALLAX_NEAR);
    createParallaxLayer("assets/sprites/parallax/4.png", 80.0f, RenderLayer::PARALLAX_NEAR);

    createStarField(30, 5.0f, 15.0f, RenderLayer::PARALLAX_FAR);
    createStarField(20, 100.0f, 150.0f, RenderLayer::PARALLAX_NEAR);
}

void CLIENT::ParallaxSystem::createStarField(int numStars, float minSpeed, float maxSpeed, RenderLayer layer)
{
    auto* starTexture = _resourceManager->getTexture("sprites/r-typesheet1.png");

    std::uniform_real_distribution<float> xDist(0.0f, static_cast<float>(WINDOW_WIDTH));
    std::uniform_real_distribution<float> yDist(0.0f, static_cast<float>(WINDOW_HEIGHT));
    std::uniform_real_distribution<float> speedDist(minSpeed, maxSpeed);
    std::uniform_real_distribution<float> scaleDist(0.1f, 0.3f);
    std::uniform_int_distribution<int> alphaDist(100, 255);

    for (int i = 0; i < numStars; ++i) {
        uint32_t id = _entityManager->createLocalEntity(EntityType::STAR, layer);
        auto* entity = _entityManager->getEntity(id);

        if (entity && starTexture) {
            entity->sprite = sf::Sprite(*starTexture);
            entity->position = sf::Vector2f(xDist(_rng), yDist(_rng));
            entity->velocity = sf::Vector2f(-speedDist(_rng), 0);
            entity->scale = scaleDist(_rng);
            entity->looping = true;

            entity->sprite->setPosition(entity->position);
            entity->sprite->setScale(sf::Vector2f(entity->scale, entity->scale));

            sf::Color color = sf::Color::White;
            color.a = static_cast<std::uint8_t>(alphaDist(_rng));
            entity->sprite->setColor(color);
        }
    }
}

void CLIENT::ParallaxSystem::createScrollingCloud(const std::string& texturePath, float speed, float yPos, RenderLayer layer)
{
    auto* texture = _resourceManager->getTexture(texturePath);
    if (texture) {
        uint32_t id = _entityManager->createLocalEntity(EntityType::DECORATION, layer);
        auto* entity = _entityManager->getEntity(id);

        if (entity) {
            entity->sprite = sf::Sprite(*texture);
            entity->position = sf::Vector2f(WINDOW_WIDTH, yPos);
            entity->velocity = sf::Vector2f(-speed, 0);
            entity->looping = true;

            entity->sprite->setPosition(entity->position);

            sf::Color color = sf::Color::White;
            color.a = 150;
            entity->sprite->setColor(color);
        }
    }
}

void CLIENT::ParallaxSystem::createObstacle(uint32_t serverId, float x, float y, const std::string& texturePath, uint8_t size)
{
    auto* texture = _resourceManager->getTexture(texturePath);
    if (texture) {
        _entityManager->createServerEntity(serverId, EntityType::OBSTACLE, RenderLayer::OBSTACLES);
        auto* entity = _entityManager->getEntity(serverId);

        if (entity) {
            entity->sprite = sf::Sprite(*texture);
            entity->position = sf::Vector2f(x, y);
            entity->velocity = sf::Vector2f(-100.0f, 0);
            entity->sprite->setPosition(entity->position);

            switch (size) {
                case 0:
                    entity->sprite->setScale(sf::Vector2f(0.5f, 0.5f));
                    break;
                case 1:
                    entity->sprite->setScale(sf::Vector2f(1.0f, 1.0f));
                    break;
                case 2:
                    entity->sprite->setScale(sf::Vector2f(1.5f, 1.5f));
                    break;
            }
        }
    }
}
