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

#include "../keybind/KeybindManager.hpp"
#include "../keybind/KeybindMenu.hpp"

namespace CLIENT {

class Window
{
   public:
    Window(const std::string& title, unsigned int width, unsigned int height);
    ~Window();

    bool isOpen() const;
    void pollEvents();
    void clear();
    void display();

    sf::RenderWindow& getWindow();
    const std::vector<std::string>& getPendingActions() const;
    float getDeltaTime() const;

    void setKeybindComponents(KeybindManager* manager, KeybindMenu* menu);
    void setColorBlindFilter(ColorBlindFilter* filter)
    {
        _colorBlindFilter = filter;
    }

   private:
    sf::RenderWindow _window;
    sf::Clock _clock;
    float _deltaTime;

    std::vector<std::string> _pendingActions;

    KeybindManager* _keybindManager;
    KeybindMenu* _keybindMenu;
    ColorBlindFilter* _colorBlindFilter;
};

}  // namespace CLIENT
