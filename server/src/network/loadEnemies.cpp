/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** loadEnemies.cpp
*/

#include "NetworkServer.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

static std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

static std::string extractJSONValue(
    const std::string& content, const std::string& key, 
    size_t start, size_t end)
{
    size_t keyPos = content.find("\"" + key + "\"", start);
    if (keyPos >= end)
        return "";

    size_t colonPos = content.find(':', keyPos);
    if (colonPos >= end)
        return "";

    size_t commaPos = content.find(',', colonPos);
    size_t bracePos = content.find('}', colonPos);
    size_t valueEnd = (commaPos < bracePos && commaPos < end) ? commaPos : bracePos;

    return trim(content.substr(colonPos + 1, valueEnd - colonPos - 1));
}

static std::string parseJSONString(
    const std::string& content, const std::string& key, 
    size_t start, size_t end)
{
    size_t keyPos = content.find("\"" + key + "\"", start);
    if (keyPos >= end)
        return "";

    size_t colonPos = content.find(':', keyPos);
    size_t quoteStart = content.find('"', colonPos + 1);
    size_t quoteEnd = content.find('"', quoteStart + 1);

    return (quoteStart < end && quoteEnd < end)
        ? content.substr(quoteStart + 1, quoteEnd - quoteStart - 1)
        : "";
}

static std::array<float, 4> parseJSONArray(
    const std::string& content, const std::string& key, 
    size_t start, size_t end)
{
    std::array<float, 4> result = {0.0f, 0.0f, 33.0f, 36.0f};

    size_t keyPos = content.find("\"" + key + "\"", start);
    if (keyPos >= end)
        return result;

    size_t bracketStart = content.find('[', keyPos);
    size_t bracketEnd = content.find(']', bracketStart);
    if (bracketStart >= end || bracketEnd >= end)
        return result;

    std::string arrayStr = content.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
    std::stringstream ss(arrayStr);
    std::string item;
    int index = 0;

    while (std::getline(ss, item, ',') && index < 4) {
        result[index++] = std::stof(trim(item));
    }

    return result;
}

void rtype::NetworkServer::loadEnemiesFromJson(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[SERVER] ERROR: Could not open enemy file: " << filepath << std::endl;
        return;
    }

    std::string content(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();

    _enemySpawnList.clear();

    size_t entitiesPos = content.find("\"entities\"");
    if (entitiesPos == std::string::npos) {
        std::cerr << "[SERVER] No 'entities' array found in JSON" << std::endl;
        return;
    }

    size_t arrayStart = content.find('[', entitiesPos);
    size_t arrayEnd = content.rfind(']');
    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) {
        std::cerr << "[SERVER] Invalid JSON format" << std::endl;
        return;
    }

    size_t pos = arrayStart + 1;
    while (pos < arrayEnd) {
        size_t objStart = content.find('{', pos);
        if (objStart >= arrayEnd)
            break;

        size_t objEnd = content.find('}', objStart);
        if (objEnd >= arrayEnd)
            break;

        EnemySpawnData enemy;
        enemy.type = std::stoi(extractJSONValue(content, "type", objStart, objEnd));
        enemy.x = std::stof(extractJSONValue(content, "x", objStart, objEnd));
        enemy.y = std::stof(extractJSONValue(content, "y", objStart, objEnd));
        enemy.spawnTime = std::stof(extractJSONValue(content, "spawnTime", objStart, objEnd));
        enemy.spritePath = parseJSONString(content, "spritePath", objStart, objEnd);
        enemy.textureRect = parseJSONArray(content, "textureRect", objStart, objEnd);

        _enemySpawnList.push_back(enemy);

        pos = objEnd + 1;
    }

    std::sort(_enemySpawnList.begin(), _enemySpawnList.end(),
        [](const EnemySpawnData& a, const EnemySpawnData& b) {
            return a.spawnTime < b.spawnTime;
        });

    std::cout << "[SERVER] ✓ Loaded " << _enemySpawnList.size() 
              << " enemies from " << filepath << std::endl;
}

void rtype::NetworkServer::checkAndSpawnEnemies()
{
    while (_nextEnemyToSpawn < _enemySpawnList.size()) {
        const auto& enemyData = _enemySpawnList[_nextEnemyToSpawn];
        
        if (_gameTime >= enemyData.spawnTime) {
            createEnemyFromData(enemyData);
            _nextEnemyToSpawn++;
        } else {
            break;
        }
    }
}

EntityManager::Entity rtype::NetworkServer::createEnemyFromData(const EnemySpawnData& data)
{
    std::lock_guard<std::mutex> lock(_registryMutex);
    auto entity = _registry->create();

    float velocity = 3.0f;
    float acceleration = -3.0f;
    int health = 1;
    int damage = 1;
    int animSpeed = 1000;

    switch (data.type) {
        case 1:
            velocity = 3.0f;
            acceleration = -3.0f;
            health = 1;
            animSpeed = 1000;
            break;
        case 2:
            velocity = 4.0f;
            acceleration = -4.0f;
            health = 2;
            animSpeed = 800;
            break;
        case 3:
            velocity = 2.5f;
            acceleration = -2.5f;
            health = 1;
            animSpeed = 1200;
            break;
        case 4:
            velocity = 5.0f;
            acceleration = -5.0f;
            health = 3;
            animSpeed = 600;
            break;
        default:
            std::cout << "[SERVER] Warning: Unknown enemy type: " << data.type << std::endl;
            break;
    }

    _registry->emplace<GameEngine::AIControlled>(entity);
    _registry->emplace<GameEngine::Acceleration>(entity, acceleration, 0.0f);
    _registry->emplace<GameEngine::Position>(entity, data.x, data.y);
    _registry->emplace<GameEngine::Velocity>(entity, velocity);

    std::vector<vec2> rectPos;
    int frameCount = 8;
    float frameWidth = data.textureRect[2];
    
    for (int i = 0; i < frameCount; i++) {
        rectPos.push_back(vec2{
            data.textureRect[0] + i * frameWidth,
            data.textureRect[1]
        });
    }

    _registry->emplace<GameEngine::Renderable>(
        entity, 1920.0f, 1080.0f, 
        data.spritePath,
        rectPos, 
        vec2{data.textureRect[2], data.textureRect[3]}, 
        animSpeed, true);

    _registry->emplace<GameEngine::Collider>(
        entity, vec2(0.0, 0.0), 
        std::bitset<8>("10100000"),
        std::bitset<8>("01000000"), 
        vec2(data.textureRect[2], data.textureRect[3]));

    _registry->emplace<GameEngine::Domain>(
        entity, 5.0f, 0.0f, 1920.0f, 1080.0);

    _registry->emplace<GameEngine::Health>(entity, health, health);
    _registry->emplace<GameEngine::Damage>(entity, damage);

    std::cout << "[SERVER] ✓ Spawned enemy type " << data.type 
              << " at (" << data.x << ", " << data.y << ")" 
              << " | time=" << data.spawnTime << "s" << std::endl;

    return entity;
}
