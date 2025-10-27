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
#include <sstream>

CLIENT::MapEditor::MapEditor()
    : _enabled(false), _selectedEntity(EntityTemplate::ENEMY_TYPE1),
      _currentSpawnTime(0.0f), _selectedEntityIndex(-1), _gridSize(32.0f),
      _snapToGrid(true), _cameraPos(0.0f, 0.0f), _zoom(1.0f),
      _isDragging(false), _draggedEntityIndex(-1)
{
    _editorView.setSize(sf::Vector2f(1920.0f, 1080.0f));
    _editorView.setCenter(sf::Vector2f(960.0f, 540.0f));
}

void CLIENT::MapEditor::update(float deltaTime)
{
    if (_enabled)
        _currentSpawnTime += deltaTime;
}

void CLIENT::MapEditor::render(sf::RenderWindow& window)
{
    if (!_enabled) return;

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

    float startX = std::floor((center.x - size.x / 2.0f) / _gridSize) * _gridSize;
    float endX = std::ceil((center.x + size.x / 2.0f) / _gridSize) * _gridSize;
    float startY = std::floor((center.y - size.y / 2.0f) / _gridSize) * _gridSize;
    float endY = std::ceil((center.y + size.y / 2.0f) / _gridSize) * _gridSize;

    std::vector<sf::Vertex> lines;
    sf::Color gridColor(50, 50, 50, 100);

    for (float x = startX; x <= endX; x += _gridSize) {
        sf::Vertex v1, v2;
        v1.position = sf::Vector2f(x, startY);
        v1.color = gridColor;
        v2.position = sf::Vector2f(x, endY);
        v2.color = gridColor;
        lines.push_back(v1);
        lines.push_back(v2);
    }

    for (float y = startY; y <= endY; y += _gridSize) {
        sf::Vertex v1, v2;
        v1.position = sf::Vector2f(startX, y);
        v1.color = gridColor;
        v2.position = sf::Vector2f(endX, y);
        v2.color = gridColor;
        lines.push_back(v1);
        lines.push_back(v2);
    }

    window.draw(lines.data(), lines.size(), sf::PrimitiveType::Lines);
}

void CLIENT::MapEditor::renderEntities(sf::RenderWindow& window)
{
    for (size_t i = 0; i < _entities.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == _selectedEntityIndex);
        renderEntity(window, _entities[i], isSelected);
    }
}

void CLIENT::MapEditor::renderEntity(sf::RenderWindow& window, const MapEntity& entity, bool selected)
{
    auto& rm = ResourceManager::getInstance();
    sf::Texture* texture = loadTexture(rm, entity.spritePath);

    if (texture) {
        renderEntitySprite(window, entity, texture, selected);
    } else {
        renderEntityFallback(window, entity, selected);
    }
}

void CLIENT::MapEditor::renderEntitySprite(sf::RenderWindow& window, const MapEntity& entity, 
                                          sf::Texture* texture, bool selected)
{
    sf::Sprite sprite(*texture);
    sprite.setTextureRect(entity.textureRect);
    sprite.setPosition(sf::Vector2f(
        entity.x - entity.textureRect.size.x / 2.0f,
        entity.y - entity.textureRect.size.y / 2.0f));
    sprite.setScale(sf::Vector2f(2.0f, 2.0f));
    window.draw(sprite);

    if (selected) {
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
}

void CLIENT::MapEditor::renderEntityFallback(sf::RenderWindow& window, const MapEntity& entity, bool selected)
{
    sf::CircleShape shape(16.0f);
    shape.setPosition(sf::Vector2f(entity.x - 16.0f, entity.y - 16.0f));
    shape.setFillColor(getEntityColor(entity.type));

    if (selected) {
        shape.setOutlineThickness(3.0f);
        shape.setOutlineColor(sf::Color::Yellow);
    }

    window.draw(shape);
}

void CLIENT::MapEditor::renderPreview(sf::RenderWindow& window)
{
    auto& rm = ResourceManager::getInstance();
    std::string spritePath;
    sf::IntRect textureRect = getEntityTemplateData(_selectedEntity, spritePath);
    sf::Texture* texture = loadTexture(rm, spritePath);

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
        previewShape.setPosition(sf::Vector2f(_mouseWorldPos.x - 16.0f, _mouseWorldPos.y - 16.0f));
        sf::Color color = getEntityColor(_selectedEntity);
        previewShape.setFillColor(sf::Color(color.r, color.g, color.b, 128));
        previewShape.setOutlineThickness(2.0f);
        previewShape.setOutlineColor(sf::Color::White);
        window.draw(previewShape);
    }
}


void CLIENT::MapEditor::renderUI()
{
    if (!_enabled) return;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Map Editor##editor", &_enabled, ImGuiWindowFlags_NoMove);

    renderUIHeader();
    renderUIEntitySelector();
    renderUIGridSettings();
    renderUICameraControls();
    renderUIEntityList();
    renderUISelectedEntity();
    renderUIFileControls();
    renderUIHelp();

    ImGui::End();
}

void CLIENT::MapEditor::renderUIHeader()
{
    ImGui::Text("R-Type Level Editor");
    ImGui::Separator();
}

void CLIENT::MapEditor::renderUIEntitySelector()
{
    ImGui::Text("Entity Type:");
    const char* entityTypes[] = {
        "Player Spawn", "Enemy Type 1 (sheet5)", "Enemy Type 2 (sheet9)",
        "Enemy Type 3 (sheet10)", "Enemy Type 4 (sheet11)", "Obstacle", "Power Up"};
    int currentType = static_cast<int>(_selectedEntity);
    if (ImGui::Combo("Type", &currentType, entityTypes, IM_ARRAYSIZE(entityTypes))) {
        _selectedEntity = static_cast<EntityTemplate>(currentType);
    }
    ImGui::Separator();
}

void CLIENT::MapEditor::renderUIGridSettings()
{
    ImGui::Checkbox("Snap to Grid", &_snapToGrid);
    ImGui::SliderFloat("Grid Size", &_gridSize, 16.0f, 128.0f);
    ImGui::Separator();
}

void CLIENT::MapEditor::renderUICameraControls()
{
    ImGui::Text("Camera:");
    if (ImGui::Button("Reset View")) {
        _editorView.setCenter(sf::Vector2f(960.0f, 540.0f));
        _zoom = 1.0f;
    }
    ImGui::SliderFloat("Zoom", &_zoom, 0.1f, 3.0f);
    ImGui::Separator();
}

void CLIENT::MapEditor::renderUIEntityList()
{
    ImGui::Text("Entities (%zu):", _entities.size());
    if (ImGui::BeginListBox("##entities", ImVec2(-1, 150))) {
        for (size_t i = 0; i < _entities.size(); ++i) {
            const auto& entity = _entities[i];
            char label[128];
            snprintf(label, sizeof(label), "%s @ (%.0f, %.0f) t=%.2fs",
                getEntityName(entity.type).c_str(), entity.x, entity.y, entity.spawnTime);

            if (ImGui::Selectable(label, _selectedEntityIndex == static_cast<int>(i))) {
                _selectedEntityIndex = static_cast<int>(i);
            }
        }
        ImGui::EndListBox();
    }
}

void CLIENT::MapEditor::renderUISelectedEntity()
{
    if (_selectedEntityIndex >= 0 && _selectedEntityIndex < static_cast<int>(_entities.size())) {
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
}

void CLIENT::MapEditor::renderUIFileControls()
{
    ImGui::Separator();
    static char filename[256] = "map_level1.json";
    ImGui::InputText("Filename", filename, sizeof(filename));

    if (ImGui::Button("Save Map")) saveMap(filename);
    ImGui::SameLine();
    if (ImGui::Button("Load Map")) loadMap(filename);
    ImGui::SameLine();
    if (ImGui::Button("Clear All")) clearMap();
}

void CLIENT::MapEditor::renderUIHelp()
{
    ImGui::Separator();
    ImGui::Text("Controls:");
    ImGui::BulletText("Left Click: Place entity");
    ImGui::BulletText("Right Click: Delete entity");
}


void CLIENT::MapEditor::handleMouseInput(sf::RenderWindow& window)
{
    if (!_enabled) return;

    updateMousePosition(window);
    handleLeftClick();
    handleRightClick();
}

void CLIENT::MapEditor::updateMousePosition(sf::RenderWindow& window)
{
    sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
    _mouseWorldPos = window.mapPixelToCoords(mousePixel, _editorView);

    if (_snapToGrid) {
        _mouseWorldPos.x = std::round(_mouseWorldPos.x / _gridSize) * _gridSize;
        _mouseWorldPos.y = std::round(_mouseWorldPos.y / _gridSize) * _gridSize;
    }
}

void CLIENT::MapEditor::handleLeftClick()
{
    static bool leftMouseWasPressed = false;
    bool leftMouseIsPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) 
                            && !ImGui::GetIO().WantCaptureMouse;

    if (leftMouseIsPressed && !leftMouseWasPressed) {
        placeEntity();
    }
    leftMouseWasPressed = leftMouseIsPressed;
}

void CLIENT::MapEditor::handleRightClick()
{
    static bool rightMouseWasPressed = false;
    bool rightMouseIsPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) 
                             && !ImGui::GetIO().WantCaptureMouse;

    if (rightMouseIsPressed && !rightMouseWasPressed) {
        deleteEntityAtMouse();
    }
    rightMouseWasPressed = rightMouseIsPressed;
}

void CLIENT::MapEditor::placeEntity()
{
    MapEntity entity;
    entity.type = _selectedEntity;
    entity.x = _mouseWorldPos.x;
    entity.y = _mouseWorldPos.y;
    entity.spawnTime = _currentSpawnTime;

    std::string spritePath;
    entity.textureRect = getEntityTemplateData(_selectedEntity, spritePath);
    entity.spritePath = spritePath;

    _entities.push_back(entity);
}

void CLIENT::MapEditor::deleteEntityAtMouse()
{
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

sf::IntRect CLIENT::MapEditor::getEntityTemplateData(EntityTemplate type, std::string& spritePath) const
{
    switch (type) {
        case EntityTemplate::ENEMY_TYPE1:
            spritePath = "assets/sprites/r-typesheet5.png";
            return sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(33, 36));
        case EntityTemplate::ENEMY_TYPE2:
            spritePath = "assets/sprites/r-typesheet9.png";
            return sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(55, 54));
        case EntityTemplate::ENEMY_TYPE3:
            spritePath = "assets/sprites/r-typesheet10.png";
            return sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(33, 30));
        case EntityTemplate::ENEMY_TYPE4:
            spritePath = "assets/sprites/r-typesheet11.png";
            return sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(33, 34));
        default:
            spritePath = "assets/sprites/r-typesheet42.png";
            return sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(33, 36));
    }
}

sf::Texture* CLIENT::MapEditor::loadTexture(ResourceManager& rm, const std::string& path) const
{
    sf::Texture* texture = rm.getTexture(path);
    if (!texture && path.find("assets/") == 0) {
        texture = rm.getTexture(path.substr(7));
    }
    return texture;
}

std::string CLIENT::MapEditor::getEntityName(EntityTemplate type) const
{
    switch (type) {
        case EntityTemplate::PLAYER_SPAWN: return "Player Spawn";
        case EntityTemplate::ENEMY_TYPE1: return "Enemy T1";
        case EntityTemplate::ENEMY_TYPE2: return "Enemy T2";
        case EntityTemplate::ENEMY_TYPE3: return "Enemy T3";
        case EntityTemplate::ENEMY_TYPE4: return "Enemy T4";
        case EntityTemplate::OBSTACLE: return "Obstacle";
        case EntityTemplate::POWER_UP: return "Power Up";
        default: return "Unknown";
    }
}

sf::Color CLIENT::MapEditor::getEntityColor(EntityTemplate type) const
{
    switch (type) {
        case EntityTemplate::PLAYER_SPAWN: return sf::Color::Green;
        case EntityTemplate::ENEMY_TYPE1: return sf::Color::Red;
        case EntityTemplate::ENEMY_TYPE2: return sf::Color(255, 100, 100);
        case EntityTemplate::ENEMY_TYPE3: return sf::Color(200, 50, 50);
        case EntityTemplate::ENEMY_TYPE4: return sf::Color(150, 0, 0);
        case EntityTemplate::OBSTACLE: return sf::Color::Blue;
        case EntityTemplate::POWER_UP: return sf::Color::Yellow;
        default: return sf::Color::White;
    }
}

void CLIENT::MapEditor::saveMap(const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to save map: " << filename << std::endl;
        return;
    }

    file << "{\n  \"entities\": [\n";

    for (size_t i = 0; i < _entities.size(); ++i) {
        const auto& e = _entities[i];
        file << "    {\n"
             << "      \"type\": " << static_cast<int>(e.type) << ",\n"
             << "      \"x\": " << e.x << ",\n"
             << "      \"y\": " << e.y << ",\n"
             << "      \"spawnTime\": " << e.spawnTime << ",\n"
             << "      \"spritePath\": \"" << e.spritePath << "\",\n"
             << "      \"textureRect\": [" << e.textureRect.position.x << ", "
             << e.textureRect.position.y << ", " << e.textureRect.size.x << ", "
             << e.textureRect.size.y << "]\n"
             << "    }" << (i < _entities.size() - 1 ? "," : "") << "\n";
    }

    file << "  ]\n}\n";
    file.close();
    std::cout << "Map saved to " << filename << " (" << _entities.size() << " entities)\n";
}

void CLIENT::MapEditor::loadMap(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to load map: " << filename << std::endl;
        return;
    }

    _entities.clear();
    _selectedEntityIndex = -1;

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    parseJSONEntities(content);

    if (!_entities.empty()) {
        _currentSpawnTime = std::max_element(_entities.begin(), _entities.end(),
            [](const MapEntity& a, const MapEntity& b) { return a.spawnTime < b.spawnTime; })->spawnTime;
    }

    std::cout << "Map loaded from " << filename << " (" << _entities.size() << " entities)\n";
}

void CLIENT::MapEditor::parseJSONEntities(const std::string& content)
{
    size_t entitiesPos = content.find("\"entities\"");
    if (entitiesPos == std::string::npos) return;

    size_t arrayStart = content.find('[', entitiesPos);
    size_t arrayEnd = content.rfind(']');
    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) return;

    size_t pos = arrayStart + 1;
    while (pos < arrayEnd) {
        size_t objStart = content.find('{', pos);
        if (objStart >= arrayEnd) break;
        
        size_t objEnd = content.find('}', objStart);
        if (objEnd >= arrayEnd) break;

        MapEntity entity = parseJSONEntity(content, objStart, objEnd);
        _entities.push_back(entity);
        
        pos = objEnd + 1;
    }
}

CLIENT::MapEntity CLIENT::MapEditor::parseJSONEntity(const std::string& content, size_t start, size_t end)
{
    MapEntity entity;
    
    entity.type = static_cast<EntityTemplate>(parseJSONInt(content, "type", start, end));
    entity.x = parseJSONFloat(content, "x", start, end);
    entity.y = parseJSONFloat(content, "y", start, end);
    entity.spawnTime = parseJSONFloat(content, "spawnTime", start, end);
    entity.spritePath = parseJSONString(content, "spritePath", start, end);
    entity.textureRect = parseJSONTextureRect(content, start, end);
    
    return entity;
}

int CLIENT::MapEditor::parseJSONInt(const std::string& content, const std::string& key, size_t start, size_t end)
{
    std::string value = extractJSONValue(content, key, start, end);
    return value.empty() ? 0 : std::stoi(value);
}

float CLIENT::MapEditor::parseJSONFloat(const std::string& content, const std::string& key, size_t start, size_t end)
{
    std::string value = extractJSONValue(content, key, start, end);
    return value.empty() ? 0.0f : std::stof(value);
}

std::string CLIENT::MapEditor::parseJSONString(const std::string& content, const std::string& key, size_t start, size_t end)
{
    size_t keyPos = content.find("\"" + key + "\"", start);
    if (keyPos >= end) return "";
    
    size_t colonPos = content.find(':', keyPos);
    size_t quoteStart = content.find('"', colonPos + 1);
    size_t quoteEnd = content.find('"', quoteStart + 1);
    
    return (quoteStart < end && quoteEnd < end) ? content.substr(quoteStart + 1, quoteEnd - quoteStart - 1) : "";
}

sf::IntRect CLIENT::MapEditor::parseJSONTextureRect(const std::string& content, size_t start, size_t end)
{
    size_t rectPos = content.find("\"textureRect\"", start);
    if (rectPos >= end) return sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(32, 32));
    
    size_t bracketStart = content.find('[', rectPos);
    size_t bracketEnd = content.find(']', bracketStart);
    if (bracketStart >= end || bracketEnd >= end) return sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(32, 32));
    
    std::string rectStr = content.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
    std::vector<int> values;
    std::stringstream ss(rectStr);
    std::string item;
    
    while (std::getline(ss, item, ',') && values.size() < 4) {
        values.push_back(static_cast<int>(std::stof(trim(item))));
    }
    
    return values.size() >= 4 ? sf::IntRect(sf::Vector2i(values[0], values[1]), sf::Vector2i(values[2], values[3]))
                              : sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(32, 32));
}

std::string CLIENT::MapEditor::extractJSONValue(const std::string& content, const std::string& key, size_t start, size_t end)
{
    size_t keyPos = content.find("\"" + key + "\"", start);
    if (keyPos >= end) return "";
    
    size_t colonPos = content.find(':', keyPos);
    if (colonPos >= end) return "";
    
    size_t commaPos = content.find(',', colonPos);
    size_t bracePos = content.find('}', colonPos);
    size_t valueEnd = (commaPos < bracePos && commaPos < end) ? commaPos : bracePos;
    
    return trim(content.substr(colonPos + 1, valueEnd - colonPos - 1));
}

std::string CLIENT::MapEditor::trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

void CLIENT::MapEditor::clearMap()
{
    _entities.clear();
    _selectedEntityIndex = -1;
    _currentSpawnTime = 0.0f;
    std::cout << "Map cleared\n";
}