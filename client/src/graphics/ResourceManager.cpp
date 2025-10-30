/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** ResourceManager.cpp
*/

#include "ResourceManager.hpp"

#include <iostream>

/**
 * @brief Retrieves the singleton instance of the ResourceManager.
 *
 * @return Reference to the singleton ResourceManager instance.
 *
 * @details
 * Ensures that only one instance of ResourceManager exists throughout the
 * program. Uses static local variable initialization.
 */
CLIENT::ResourceManager &CLIENT::ResourceManager::getInstance()
{
    static ResourceManager instance;
    return instance;
}

/**
 * @brief Default constructor for ResourceManager.
 *
 * @details
 * Initializes the internal texture storage. No textures are loaded by default.
 */
CLIENT::ResourceManager::ResourceManager() = default;

/**
 * @brief Destructor for ResourceManager.
 *
 * @details
 * Clears all loaded textures automatically upon destruction.
 */
CLIENT::ResourceManager::~ResourceManager() = default;

/**
 * @brief Loads a texture from a file and stores it with a given ID.
 *
 * @param id Unique identifier for the texture.
 * @param filepath Path to the texture file.
 * @return True if the texture was successfully loaded, false otherwise.
 *
 * @details
 * If the texture cannot be loaded from the specified file, an error message
 * is printed. Successfully loaded textures are stored in the internal map.
 */
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

/**
 * @brief Retrieves a texture by its ID.
 *
 * @param id The ID of the texture to retrieve.
 * @return Pointer to the sf::Texture if found, nullptr otherwise.
 *
 * @details
 * If the texture does not exist in the manager, an error message is printed.
 */
sf::Texture *CLIENT::ResourceManager::getTexture(const std::string &id)
{
    auto it = _textures.find(id);
    if (it != _textures.end())
        return it->second.get();
    std::cerr << "Texture not found: " << id << "\n";
    return nullptr;
}

/**
 * @brief Clears all loaded textures from the manager.
 *
 * @details
 * All textures stored in the internal map are destroyed, freeing memory.
 */
void CLIENT::ResourceManager::clear()
{
    _textures.clear();
}
