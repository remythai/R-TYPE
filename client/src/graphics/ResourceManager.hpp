/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** ResourceManager.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <memory>

namespace CLIENT {

class ResourceManager {
public:
    static ResourceManager &getInstance();

    bool loadTexture(const std::string &id, const std::string &filepath);
    sf::Texture *getTexture(const std::string &id);
    void clear();

private:
    ResourceManager();
    ~ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::map<std::string, std::unique_ptr<sf::Texture>> _textures;
};

} // namespace CLIENT
