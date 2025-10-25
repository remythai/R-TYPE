/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** ResourceManager.cpp
*/

#include "ResourceManager.hpp"

#include <iostream>

CLIENT::ResourceManager &CLIENT::ResourceManager::getInstance()
{
    static ResourceManager instance;
    return instance;
}

CLIENT::ResourceManager::ResourceManager() = default;
CLIENT::ResourceManager::~ResourceManager() = default;

bool CLIENT::ResourceManager::loadTexture(
    const std::string &id, const std::string &filepath)
{
    auto texture = std::make_unique<sf::Texture>();
    if (!texture->loadFromFile(filepath)) {
        std::cerr << "Failed to load texture: " << filepath << "\n";
        return false;
    }
    _textures[id] = std::move(texture);
    std::cout << "Texture loaded: " << id << " from " << filepath << "\n";
    return true;
}

sf::Texture *CLIENT::ResourceManager::getTexture(const std::string &id)
{
    auto it = _textures.find(id);
    if (it != _textures.end())
        return it->second.get();
    std::cerr << "Texture not found: " << id << "\n";
    return nullptr;
}

void CLIENT::ResourceManager::clear()
{
    _textures.clear();
}
