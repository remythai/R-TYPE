/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** MapEditor.hpp
*/

#pragma once

#include <imgui-SFML.h>
#include <imgui.h>

#include <SFML/Graphics.hpp>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "../graphics/ResourceManager.hpp"

namespace CLIENT {

enum class EntityTemplate
{
    PLAYER_SPAWN,
    ENEMY_TYPE1,
    ENEMY_TYPE2,
    ENEMY_TYPE3,
    ENEMY_TYPE4,
    OBSTACLE,
    POWER_UP
};

struct MapEntity
{
    EntityTemplate type;
    float x;
    float y;
    float spawnTime;
    std::string spritePath;
    sf::IntRect textureRect;
};

class MapEditor
{
   public:
    MapEditor();
    ~MapEditor() = default;

    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    void renderUI();

    bool isEnabled() const
    {
        return _enabled;
    }
    void setEnabled(bool enabled)
    {
        _enabled = enabled;
    }
    void toggle()
    {
        _enabled = !_enabled;
    }

    void saveMap(const std::string& filename);
    void loadMap(const std::string& filename);
    void clearMap();

    const std::vector<MapEntity>& getEntities() const
    {
        return _entities;
    }
    void handleMouseInput(sf::RenderWindow& window);
    void renderGrid(sf::RenderWindow& window);
    void renderEntities(sf::RenderWindow& window);
    void renderPreview(sf::RenderWindow& window);

    std::string getEntityName(EntityTemplate type) const;
    sf::Color getEntityColor(EntityTemplate type) const;

   private:
    bool _enabled;
    EntityTemplate _selectedEntity;
    float _currentSpawnTime;

    std::vector<MapEntity> _entities;
    int _selectedEntityIndex;

    float _gridSize;
    bool _snapToGrid;

    sf::View _editorView;
    sf::Vector2f _cameraPos;
    float _zoom;

    sf::Vector2f _mouseWorldPos;
    bool _isDragging;
    int _draggedEntityIndex;
};

}  // namespace CLIENT