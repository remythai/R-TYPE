/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** KeybindMenu.cpp
*/

#include "KeybindMenu.hpp"
#include <cmath>
#include <iostream>

CLIENT::KeybindMenu::KeybindMenu(KeybindManager& keybindManager)
    : _keybindManager(keybindManager),
      _isOpen(false),
      _font(std::make_shared<sf::Font>()),
      _titleText(*_font),
      _instructionText(*_font),
      _saveText(*_font),
      _resetText(*_font),
      _closeText(*_font),
      _selectedIndex(-1),
      _blinkTimer(0.0f)
{
    if (!_font->openFromFile("assets/fonts/BoldPixels.ttf")) {
        std::cerr << "Failed to load font for keybind menu" << std::endl;
    }
    initializeUI();
}

void CLIENT::KeybindMenu::initializeUI()
{
    _background.setSize(sf::Vector2f(800, 600));
    _background.setPosition(sf::Vector2f(260, 140));
    _background.setFillColor(sf::Color(20, 20, 40, 230));
    _background.setOutlineColor(sf::Color::White);
    _background.setOutlineThickness(3.0f);

    _titleText.setString("KEYBIND SETTINGS");
    _titleText.setCharacterSize(40);
    _titleText.setFillColor(sf::Color::White);
    _titleText.setPosition(sf::Vector2f(480, 160));

    _instructionText.setString("Click on a key to rebind it, or press ESC to cancel");
    _instructionText.setCharacterSize(18);
    _instructionText.setFillColor(sf::Color(200, 200, 200));
    _instructionText.setPosition(sf::Vector2f(320, 210));

    _saveButton.setSize(sf::Vector2f(180, 50));
    _saveButton.setPosition(sf::Vector2f(300, 650));
    _saveButton.setFillColor(sf::Color(50, 150, 50));
    _saveButton.setOutlineColor(sf::Color::White);
    _saveButton.setOutlineThickness(2.0f);

    _saveText.setString("SAVE");
    _saveText.setCharacterSize(24);
    _saveText.setFillColor(sf::Color::White);
    _saveText.setPosition(sf::Vector2f(350, 662));

    _resetButton.setSize(sf::Vector2f(180, 50));
    _resetButton.setPosition(sf::Vector2f(520, 650));
    _resetButton.setFillColor(sf::Color(150, 100, 50));
    _resetButton.setOutlineColor(sf::Color::White);
    _resetButton.setOutlineThickness(2.0f);

    _resetText.setString("RESET");
    _resetText.setCharacterSize(24);
    _resetText.setFillColor(sf::Color::White);
    _resetText.setPosition(sf::Vector2f(560, 662));

    _closeButton.setSize(sf::Vector2f(180, 50));
    _closeButton.setPosition(sf::Vector2f(740, 650));
    _closeButton.setFillColor(sf::Color(150, 50, 50));
    _closeButton.setOutlineColor(sf::Color::White);
    _closeButton.setOutlineThickness(2.0f);

    _closeText.setString("CLOSE");
    _closeText.setCharacterSize(24);
    _closeText.setFillColor(sf::Color::White);
    _closeText.setPosition(sf::Vector2f(780, 662));

    updateUI();
}

void CLIENT::KeybindMenu::updateUI()
{
    _actionTexts.clear();
    _keyTexts.clear();
    _buttons.clear();

    auto actions = _keybindManager.getAllActions();
    float startY = 270;
    float spacing = 60;

    for (size_t i = 0; i < actions.size(); ++i) {
        GameAction action = actions[i];
        
        sf::Text actionText(*_font);
        actionText.setString(_keybindManager.getActionName(action));
        actionText.setCharacterSize(24);
        actionText.setFillColor(sf::Color::White);
        actionText.setPosition(sf::Vector2f(320, startY + i * spacing));
        _actionTexts.push_back(actionText);

        sf::RectangleShape button;
        button.setSize(sf::Vector2f(200, 45));
        button.setPosition(sf::Vector2f(650, startY + i * spacing - 5));
        button.setFillColor(sf::Color(60, 60, 80));
        button.setOutlineColor(sf::Color(150, 150, 150));
        button.setOutlineThickness(2.0f);
        _buttons.push_back(button);

        // Key text
        sf::Text keyText(*_font);
        sf::Keyboard::Key key = _keybindManager.getKeybind(action);
        keyText.setString(_keybindManager.getKeyName(key));
        keyText.setCharacterSize(22);
        keyText.setFillColor(sf::Color::White);
        keyText.setPosition(sf::Vector2f(700, startY + i * spacing));
        _keyTexts.push_back(keyText);
    }
}

void CLIENT::KeybindMenu::open()
{
    _isOpen = true;
    _waitingForKey.reset();
    updateUI();
}

void CLIENT::KeybindMenu::close()
{
    _isOpen = false;
    _waitingForKey.reset();
}

bool CLIENT::KeybindMenu::isOpen() const
{
    return _isOpen;
}

void CLIENT::KeybindMenu::handleEvent(const sf::Event& event)
{
    if (!_isOpen)
        return;

    if (event.is<sf::Event::KeyPressed>()) {
        const auto* keyEvent = event.getIf<sf::Event::KeyPressed>();
        
        if (_waitingForKey.has_value()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                _waitingForKey.reset();
                updateUI();
            } else {
                _keybindManager.setKeybind(_waitingForKey.value(), keyEvent->code);
                _waitingForKey.reset();
                updateUI();
            }
        } else {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                close();
            }
        }
    }

    if (event.is<sf::Event::MouseButtonPressed>()) {
        const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            handleMouseClick(mouseEvent->position);
        }
    }
}

void CLIENT::KeybindMenu::handleMouseClick(const sf::Vector2i& mousePos)
{
    auto actions = _keybindManager.getAllActions();
    for (size_t i = 0; i < _buttons.size(); ++i) {
        if (isPointInRect(mousePos, _buttons[i])) {
            _waitingForKey = actions[i];
            return;
        }
    }

    if (isPointInRect(mousePos, _saveButton)) {
        _keybindManager.saveToFile("keybinds.cfg");
    }

    if (isPointInRect(mousePos, _resetButton)) {
        _keybindManager.resetToDefaults();
        updateUI();
    }

    if (isPointInRect(mousePos, _closeButton)) {
        close();
    }
}

bool CLIENT::KeybindMenu::isPointInRect(const sf::Vector2i& point, const sf::RectangleShape& rect) const
{
    sf::FloatRect bounds = rect.getGlobalBounds();
    return bounds.contains(sf::Vector2f(static_cast<float>(point.x), static_cast<float>(point.y)));
}

void CLIENT::KeybindMenu::update(float deltaTime)
{
    if (!_isOpen)
        return;

    _blinkTimer += deltaTime;
}

void CLIENT::KeybindMenu::render(sf::RenderWindow& window)
{
    if (!_isOpen)
        return;

    window.draw(_background);
    window.draw(_titleText);
    window.draw(_instructionText);

    for (size_t i = 0; i < _buttons.size(); ++i) {
        sf::RectangleShape button = _buttons[i];
        
        if (_waitingForKey.has_value()) {
            auto actions = _keybindManager.getAllActions();
            if (i < actions.size() && actions[i] == _waitingForKey.value()) {
                int alpha = static_cast<int>(std::abs(std::sin(_blinkTimer * 5.0f)) * 100 + 155);
                button.setFillColor(sf::Color(100, 150, 255, alpha));
            }
        }
        
        window.draw(button);
        window.draw(_actionTexts[i]);
        
        if (_waitingForKey.has_value()) {
            auto actions = _keybindManager.getAllActions();
            if (i < actions.size() && actions[i] == _waitingForKey.value()) {
                sf::Text waitText(*_font);
                waitText.setString("Press a key...");
                waitText.setCharacterSize(20);
                waitText.setFillColor(sf::Color::Yellow);
                waitText.setPosition(_keyTexts[i].getPosition());
                window.draw(waitText);
            } else {
                window.draw(_keyTexts[i]);
            }
        } else {
            window.draw(_keyTexts[i]);
        }
    }

    window.draw(_saveButton);
    window.draw(_saveText);
    window.draw(_resetButton);
    window.draw(_resetText);
    window.draw(_closeButton);
    window.draw(_closeText);
}
