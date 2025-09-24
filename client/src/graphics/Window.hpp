/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Window.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <string>

namespace CLIENT {

class Window {
public:
    Window(const std::string &title, unsigned int width, unsigned int height);
    ~Window();

    bool isOpen() const;

    void pollEvents();

    void clear();
    void display();

    sf::RenderWindow &getHandle();

private:
    sf::RenderWindow _window;
};

} // namespace Engine
