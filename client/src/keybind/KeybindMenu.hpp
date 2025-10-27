/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** KeybindMenu.hpp
*/

#pragma once

#include "KeybindManager.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>

namespace CLIENT {

class KeybindMenu {
public:
    KeybindMenu(KeybindManager& keybindManager);
    ~KeybindMenu() = default;

    void open();
    void close();
    bool isOpen() const;
    
    void handleEvent(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

private:
    KeybindManager& _keybindManager;
    bool _isOpen;
    
    std::optional<GameAction> _waitingForKey;
    
    std::shared_ptr<sf::Font> _font;
    std::vector<sf::Text> _actionTexts;
    std::vector<sf::Text> _keyTexts;
    std::vector<sf::RectangleShape> _buttons;
    sf::Text _titleText;
    sf::Text _instructionText;
    sf::RectangleShape _background;
    
    sf::RectangleShape _saveButton;
    sf::Text _saveText;
    sf::RectangleShape _resetButton;
    sf::Text _resetText;
    sf::RectangleShape _closeButton;
    sf::Text _closeText;
    
    int _selectedIndex;
    float _blinkTimer;
    
    void initializeUI();
    void updateUI();
    void handleMouseClick(const sf::Vector2i& mousePos);
    bool isPointInRect(const sf::Vector2i& point, const sf::RectangleShape& rect) const;
};

} // namespace CLIENT