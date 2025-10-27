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
    void handleMouseInput(sf::RenderWindow& window);

    [[nodiscard]] bool isEnabled() const
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
    [[nodiscard]] const std::vector<MapEntity>& getEntities() const
    {
        return _entities;
    }

    void saveMap(const std::string& filename);
    void loadMap(const std::string& filename);
    void clearMap();

   private:
    void renderGrid(sf::RenderWindow& window);
    void renderEntities(sf::RenderWindow& window);
    void renderEntity(
        sf::RenderWindow& window, const MapEntity& entity, bool selected);
    void renderEntitySprite(
        sf::RenderWindow& window, const MapEntity& entity, sf::Texture* texture,
        bool selected);
    void renderEntityFallback(
        sf::RenderWindow& window, const MapEntity& entity, bool selected);
    void renderPreview(sf::RenderWindow& window);

    void renderUIHeader();
    void renderUIEntitySelector();
    void renderUIGridSettings();
    void renderUICameraControls();
    void renderUIEntityList();
    void renderUISelectedEntity();
    void renderUIFileControls();
    void renderUIHelp();

    void updateMousePosition(sf::RenderWindow& window);
    void handleLeftClick();
    void handleRightClick();
    void placeEntity();
    void deleteEntityAtMouse();

    sf::IntRect getEntityTemplateData(
        EntityTemplate type, std::string& spritePath) const;
    sf::Texture* loadTexture(
        ResourceManager& rm, const std::string& path) const;
    [[nodiscard]] std::string getEntityName(EntityTemplate type) const;
    [[nodiscard]] sf::Color getEntityColor(EntityTemplate type) const;

    void parseJSONEntities(const std::string& content);
    MapEntity parseJSONEntity(
        const std::string& content, size_t start, size_t end);
    int parseJSONInt(
        const std::string& content, const std::string& key, size_t start,
        size_t end);
    float parseJSONFloat(
        const std::string& content, const std::string& key, size_t start,
        size_t end);
    std::string parseJSONString(
        const std::string& content, const std::string& key, size_t start,
        size_t end);
    sf::IntRect parseJSONTextureRect(
        const std::string& content, size_t start, size_t end);
    std::string extractJSONValue(
        const std::string& content, const std::string& key, size_t start,
        size_t end);
    static std::string trim(const std::string& str);

    bool _enabled;
    EntityTemplate _selectedEntity;
    float _currentSpawnTime;
    int _selectedEntityIndex;

    std::vector<MapEntity> _entities;

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