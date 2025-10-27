/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** KeybindMenu.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>
#include <vector>

#include "KeybindManager.hpp"
#include "../colorBlindFilter/ColorBlindFilter.hpp"

namespace CLIENT {

class KeybindMenu {
public:
    explicit KeybindMenu(KeybindManager& keybindManager);
    ~KeybindMenu() = default;

    void open();
    void close();
    bool isOpen() const;

    void handleEvent(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderTarget& target);  // ← CHANGÉ ICI
    
    void setColorBlindFilter(ColorBlindFilter* filter) { _colorBlindFilter = filter; }

private:
    void initializeUI();
    void updateUI();
    void handleMouseClick(const sf::Vector2i& mousePos);
    bool isPointInRect(
        const sf::Vector2i& point, const sf::RectangleShape& rect) const;

    KeybindManager& _keybindManager;
    ColorBlindFilter* _colorBlindFilter;
    
    bool _isOpen;
    std::shared_ptr<sf::Font> _font;

    sf::RectangleShape _background;
    sf::Text _titleText;
    sf::Text _instructionText;

    std::vector<sf::Text> _actionTexts;
    std::vector<sf::Text> _keyTexts;
    std::vector<sf::RectangleShape> _buttons;

    sf::RectangleShape _saveButton;
    sf::Text _saveText;
    sf::RectangleShape _resetButton;
    sf::Text _resetText;
    sf::RectangleShape _closeButton;
    sf::Text _closeText;
    
    // Boutons pour le filtre daltonien
    sf::RectangleShape _colorBlindPrevButton;
    sf::RectangleShape _colorBlindNextButton;
    sf::Text _colorBlindLabel;
    sf::Text _colorBlindModeText;
    sf::Text _colorBlindPrevText;
    sf::Text _colorBlindNextText;

    std::optional<GameAction> _waitingForKey;
    float _blinkTimer;
};

} // namespace CLIENT
