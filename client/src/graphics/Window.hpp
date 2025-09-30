/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Window.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace CLIENT {

class Window {
public:
    Window(const std::string &title, unsigned int width, unsigned int height);
    ~Window();

    bool isOpen() const;
    void pollEvents();
    void clear();
    void display();

    sf::RenderWindow &getWindow();
    const std::vector<std::string> &getPendingActions() const;
    float getDeltaTime() const;

private:
    sf::RenderWindow _window;
    std::vector<std::string> _pendingActions;
    sf::Clock _clock;
    float _deltaTime;
};

} // namespace CLIENT