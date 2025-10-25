/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** MapEditor.cpp
*/

#include "MapEditor.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

CLIENT::MapEditor::MapEditor()
    : _enabled(false),
      _selectedEntity(EntityTemplate::ENEMY_TYPE1),
      _currentSpawnTime(0.0f),
      _selectedEntityIndex(-1),
      _gridSize(32.0f),
      _snapToGrid(true),
      _cameraPos(0.0f, 0.0f),
      _zoom(1.0f),
      _isDragging(false),
      _draggedEntityIndex(-1)
{
    _editorView.setSize(sf::Vector2f(1920.0f, 1080.0f));
    _editorView.setCenter(sf::Vector2f(960.0f, 540.0f));
}

void CLIENT::MapEditor::update(float deltaTime)
{
    if (!_enabled)
        return;

    _currentSpawnTime += deltaTime;
}

void CLIENT::MapEditor::render(sf::RenderWindow& window)
{
    if (!_enabled)
        return;

    sf::View originalView = window.getView();
    window.setView(_editorView);

    renderGrid(window);
    renderEntities(window);
    renderPreview(window);

    window.setView(originalView);
}

void CLIENT::MapEditor::renderGrid(sf::RenderWindow& window)
{
    sf::View view = window.getView();
    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();

    float startX =
        std::floor((center.x - size.x / 2.0f) / _gridSize) * _gridSize;
    float endX = std::ceil((center.x + size.x / 2.0f) / _gridSize) * _gridSize;
    float startY =
        std::floor((center.y - size.y / 2.0f) / _gridSize) * _gridSize;
    float endY = std::ceil((center.y + size.y / 2.0f) / _gridSize) * _gridSize;

    std::vector<sf::Vertex> lines;

    for (float x = startX; x <= endX; x += _gridSize) {
        sf::Vertex v1, v2;
        v1.position = sf::Vector2f(x, startY);
        v1.color = sf::Color(50, 50, 50, 100);
        v2.position = sf::Vector2f(x, endY);
        v2.color = sf::Color(50, 50, 50, 100);
        lines.push_back(v1);
        lines.push_back(v2);
    }

    for (float y = startY; y <= endY; y += _gridSize) {
        sf::Vertex v1, v2;
        v1.position = sf::Vector2f(startX, y);
        v1.color = sf::Color(50, 50, 50, 100);
        v2.position = sf::Vector2f(endX, y);
        v2.color = sf::Color(50, 50, 50, 100);
        lines.push_back(v1);
        lines.push_back(v2);
    }

    window.draw(lines.data(), lines.size(), sf::PrimitiveType::Lines);
}

void CLIENT::MapEditor::renderEntities(sf::RenderWindow& window)
{
    auto& rm = ResourceManager::getInstance();

    for (size_t i = 0; i < _entities.size(); ++i) {
        const auto& entity = _entities[i];

        sf::Texture* texture = rm.getTexture(entity.spritePath);
        if (!texture) {
            std::string altPath = entity.spritePath;
            if (altPath.find("assets/") == 0) {
                altPath = altPath.substr(7);
                texture = rm.getTexture(altPath);
            }
        }

        if (texture) {
            sf::Sprite sprite(*texture);
            sprite.setTextureRect(entity.textureRect);
            sprite.setPosition(sf::Vector2f(
                entity.x - entity.textureRect.size.x / 2.0f,
                entity.y - entity.textureRect.size.y / 2.0f));
            sprite.setScale(sf::Vector2f(2.0f, 2.0f));

            window.draw(sprite);

            if (static_cast<int>(i) == _selectedEntityIndex) {
                sf::RectangleShape selectionBox(sf::Vector2f(
                    entity.textureRect.size.x * 2.0f,
                    entity.textureRect.size.y * 2.0f));
                selectionBox.setPosition(sf::Vector2f(
                    entity.x - entity.textureRect.size.x,
                    entity.y - entity.textureRect.size.y));
                selectionBox.setFillColor(sf::Color::Transparent);
                selectionBox.setOutlineThickness(2.0f);
                selectionBox.setOutlineColor(sf::Color::Yellow);
                window.draw(selectionBox);
            }
        } else {
            sf::CircleShape shape(16.0f);
            shape.setPosition(sf::Vector2f(entity.x - 16.0f, entity.y - 16.0f));
            shape.setFillColor(getEntityColor(entity.type));

            if (static_cast<int>(i) == _selectedEntityIndex) {
                shape.setOutlineThickness(3.0f);
                shape.setOutlineColor(sf::Color::Yellow);
            }

            window.draw(shape);
        }
    }
}

void CLIENT::MapEditor::renderPreview(sf::RenderWindow& window)
{
    auto& rm = ResourceManager::getInstance();

    std::string spritePath;
    sf::IntRect textureRect(sf::Vector2i(0, 0), sf::Vector2i(32, 32));

    switch (_selectedEntity) {
        case EntityTemplate::ENEMY_TYPE1:
            spritePath = "assets/sprites/r-typesheet5.png";
            textureRect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(33, 36));
            break;
        case EntityTemplate::ENEMY_TYPE2:
            spritePath = "assets/sprites/r-typesheet9.png";
            textureRect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(55, 54));
            break;
        case EntityTemplate::ENEMY_TYPE3:
            spritePath = "assets/sprites/r-typesheet10.png";
            textureRect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(33, 30));
            break;
        case EntityTemplate::ENEMY_TYPE4:
            spritePath = "assets/sprites/r-typesheet11.png";
            textureRect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(33, 34));
            break;
        case EntityTemplate::PLAYER_SPAWN:
            spritePath = "assets/sprites/r-typesheet42.png";
            textureRect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(33, 36));
            break;
        default:
            spritePath = "assets/sprites/r-typesheet42.png";
            textureRect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(33, 36));
            break;
    }

    sf::Texture* texture = rm.getTexture(spritePath);
    if (!texture) {
        std::string altPath = spritePath;
        if (altPath.find("assets/") == 0) {
            altPath = altPath.substr(7);
            texture = rm.getTexture(altPath);
        }
    }

    if (texture) {
        sf::Sprite preview(*texture);
        preview.setTextureRect(textureRect);
        preview.setPosition(sf::Vector2f(
            _mouseWorldPos.x - textureRect.size.x,
            _mouseWorldPos.y - textureRect.size.y));
        preview.setScale(sf::Vector2f(2.0f, 2.0f));
        preview.setColor(sf::Color(255, 255, 255, 128));

        window.draw(preview);
    } else {
        sf::CircleShape previewShape(16.0f);
        previewShape.setPosition(
            sf::Vector2f(_mouseWorldPos.x - 16.0f, _mouseWorldPos.y - 16.0f));
        previewShape.setFillColor(sf::Color(
            getEntityColor(_selectedEntity).r,
            getEntityColor(_selectedEntity).g,
            getEntityColor(_selectedEntity).b, 128));
        previewShape.setOutlineThickness(2.0f);
        previewShape.setOutlineColor(sf::Color::White);

        window.draw(previewShape);
    }
}

void CLIENT::MapEditor::renderUI()
{
    if (!_enabled)
        return;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 700), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove;
    ImGui::Begin("Map Editor##editor", &_enabled, flags);

    ImGui::Text("R-Type Level Editor");
    ImGui::Separator();

    ImGui::Text("Entity Type:");
    const char* entityTypes[] = {
        "Player Spawn",
        "Enemy Type 1 (sheet5)",
        "Enemy Type 2 (sheet9)",
        "Enemy Type 3 (sheet10)",
        "Enemy Type 4 (sheet11)",
        "Obstacle",
        "Power Up"};
    int currentType = static_cast<int>(_selectedEntity);
    if (ImGui::Combo(
            "Type", &currentType, entityTypes, IM_ARRAYSIZE(entityTypes))) {
        _selectedEntity = static_cast<EntityTemplate>(currentType);
    }

    ImGui::Separator();

    ImGui::Checkbox("Snap to Grid", &_snapToGrid);
    ImGui::SliderFloat("Grid Size", &_gridSize, 16.0f, 128.0f);

    ImGui::Separator();

    ImGui::Text("Camera:");
    if (ImGui::Button("Reset View")) {
        _editorView.setCenter(sf::Vector2f(960.0f, 540.0f));
        _zoom = 1.0f;
    }
    ImGui::SliderFloat("Zoom", &_zoom, 0.1f, 3.0f);

    ImGui::Separator();

    ImGui::Text("Entities (%zu):", _entities.size());
    if (ImGui::BeginListBox("##entities", ImVec2(-1, 150))) {
        for (size_t i = 0; i < _entities.size(); ++i) {
            const auto& entity = _entities[i];
            char label[128];
            snprintf(
                label, sizeof(label), "%s @ (%.0f, %.0f) t=%.2fs",
                getEntityName(entity.type).c_str(), entity.x, entity.y,
                entity.spawnTime);

            if (ImGui::Selectable(
                    label, _selectedEntityIndex == static_cast<int>(i))) {
                _selectedEntityIndex = static_cast<int>(i);
            }
        }
        ImGui::EndListBox();
    }

    if (_selectedEntityIndex >= 0 &&
        _selectedEntityIndex < static_cast<int>(_entities.size())) {
        ImGui::Separator();
        ImGui::Text("Selected Entity:");
        auto& entity = _entities[_selectedEntityIndex];
        ImGui::InputFloat("X##pos", &entity.x);
        ImGui::InputFloat("Y##pos", &entity.y);
        ImGui::InputFloat("Spawn Time (seconds)", &entity.spawnTime);

        if (ImGui::Button("Delete Selected")) {
            _entities.erase(_entities.begin() + _selectedEntityIndex);
            _selectedEntityIndex = -1;
        }
    }

    ImGui::Separator();

    static char filename[256] = "map_level1.json";
    ImGui::InputText("Filename", filename, sizeof(filename));

    if (ImGui::Button("Save Map")) {
        saveMap(filename);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Map")) {
        loadMap(filename);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear All")) {
        clearMap();
    }

    ImGui::Separator();
    ImGui::Text("Controls:");
    ImGui::BulletText("Left Click: Place entity");
    ImGui::BulletText("Right Click: Delete entity");

    ImGui::End();
}

void CLIENT::MapEditor::handleMouseInput(sf::RenderWindow& window)
{
    if (!_enabled)
        return;

    sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
    _mouseWorldPos = window.mapPixelToCoords(mousePixel, _editorView);

    if (_snapToGrid) {
        _mouseWorldPos.x = std::round(_mouseWorldPos.x / _gridSize) * _gridSize;
        _mouseWorldPos.y = std::round(_mouseWorldPos.y / _gridSize) * _gridSize;
    }

    static bool leftMouseWasPressed = false;
    bool leftMouseIsPressed =
        sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) &&
        !ImGui::GetIO().WantCaptureMouse;

    if (leftMouseIsPressed && !leftMouseWasPressed) {
        MapEntity entity;
        entity.type = _selectedEntity;
        entity.x = _mouseWorldPos.x;
        entity.y = _mouseWorldPos.y;
        entity.spawnTime = _currentSpawnTime;

        switch (_selectedEntity) {
            case EntityTemplate::ENEMY_TYPE1:
                entity.spritePath = "assets/sprites/r-typesheet5.png";
                entity.textureRect =
                    sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(33.31, 36));
                break;
            case EntityTemplate::ENEMY_TYPE2:
                entity.spritePath = "assets/sprites/r-typesheet9.png";
                entity.textureRect =
                    sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(55.33, 54));
                break;
            case EntityTemplate::ENEMY_TYPE3:
                entity.spritePath = "assets/sprites/r-typesheet10.png";
                entity.textureRect =
                    sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(33.17, 30));
                break;
            case EntityTemplate::ENEMY_TYPE4:
                entity.spritePath = "assets/sprites/r-typesheet11.png";
                entity.textureRect =
                    sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(33.33, 34));
                break;
            default:
                entity.spritePath = "assets/sprites/r-typesheet42.png";
                entity.textureRect =
                    sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(33.33, 36));
                break;
        }

        _entities.push_back(entity);
    }
    leftMouseWasPressed = leftMouseIsPressed;

    static bool rightMouseWasPressed = false;
    bool rightMouseIsPressed =
        sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) &&
        !ImGui::GetIO().WantCaptureMouse;

    if (rightMouseIsPressed && !rightMouseWasPressed) {
        float closestDist = 30.0f;
        int closestIndex = -1;

        for (size_t i = 0; i < _entities.size(); ++i) {
            float dx = _entities[i].x - _mouseWorldPos.x;
            float dy = _entities[i].y - _mouseWorldPos.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < closestDist) {
                closestDist = dist;
                closestIndex = static_cast<int>(i);
            }
        }

        if (closestIndex >= 0) {
            _entities.erase(_entities.begin() + closestIndex);
            if (_selectedEntityIndex == closestIndex) {
                _selectedEntityIndex = -1;
            }
        }
    }
    rightMouseWasPressed = rightMouseIsPressed;
}

std::string CLIENT::MapEditor::getEntityName(EntityTemplate type) const
{
    switch (type) {
        case EntityTemplate::PLAYER_SPAWN:
            return "Player Spawn";
        case EntityTemplate::ENEMY_TYPE1:
            return "Enemy T1";
        case EntityTemplate::ENEMY_TYPE2:
            return "Enemy T2";
        case EntityTemplate::ENEMY_TYPE3:
            return "Enemy T3";
        case EntityTemplate::ENEMY_TYPE4:
            return "Enemy T4";
        case EntityTemplate::OBSTACLE:
            return "Obstacle";
        case EntityTemplate::POWER_UP:
            return "Power Up";
        default:
            return "Unknown";
    }
}

sf::Color CLIENT::MapEditor::getEntityColor(EntityTemplate type) const
{
    switch (type) {
        case EntityTemplate::PLAYER_SPAWN:
            return sf::Color::Green;
        case EntityTemplate::ENEMY_TYPE1:
            return sf::Color::Red;
        case EntityTemplate::ENEMY_TYPE2:
            return sf::Color(255, 100, 100);
        case EntityTemplate::ENEMY_TYPE3:
            return sf::Color(200, 50, 50);
        case EntityTemplate::ENEMY_TYPE4:
            return sf::Color(150, 0, 0);
        case EntityTemplate::OBSTACLE:
            return sf::Color::Blue;
        case EntityTemplate::POWER_UP:
            return sf::Color::Yellow;
        default:
            return sf::Color::White;
    }
}

void CLIENT::MapEditor::saveMap(const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to save map: " << filename << std::endl;
        return;
    }

    file << "{\n";
    file << "  \"entities\": [\n";

    for (size_t i = 0; i < _entities.size(); ++i) {
        const auto& entity = _entities[i];
        file << "    {\n";
        file << "      \"type\": " << static_cast<int>(entity.type) << ",\n";
        file << "      \"x\": " << entity.x << ",\n";
        file << "      \"y\": " << entity.y << ",\n";
        file << "      \"spawnTime\": " << entity.spawnTime << ",\n";
        file << "      \"spritePath\": \"" << entity.spritePath << "\",\n";
        file << "      \"textureRect\": [" << entity.textureRect.position.x
             << ", " << entity.textureRect.position.y << ", "
             << entity.textureRect.size.x << ", " << entity.textureRect.size.y
             << "]\n";
        file << "    }";
        if (i < _entities.size() - 1)
            file << ",";
        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";

    file.close();
    std::cout << "Map saved to " << filename << " (" << _entities.size()
              << " entities)\n";
}

void CLIENT::MapEditor::loadMap(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to load map: " << filename << std::endl;
        return;
    }

    _entities.clear();
    std::cout << "Map loading from " << filename
              << " (implement proper JSON parsing)\n";

    file.close();
}

void CLIENT::MapEditor::clearMap()
{
    _entities.clear();
    _selectedEntityIndex = -1;
    _currentSpawnTime = 0.0f;
    std::cout << "Map cleared\n";
}
