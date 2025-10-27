/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** KeybindManager.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <vector>

namespace CLIENT {

enum class GameAction {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    SHOOT,
    OPEN_KEYBIND_MENU
};

class KeybindManager {
public:
    KeybindManager();
    ~KeybindManager() = default;

    void setKeybind(GameAction action, sf::Keyboard::Key key);
    sf::Keyboard::Key getKeybind(GameAction action) const;
    std::string getActionName(GameAction action) const;
    std::string getKeyName(sf::Keyboard::Key key) const;
    
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);
    
    void resetToDefaults();
    
    bool isActionPressed(GameAction action) const;
    
    std::vector<GameAction> getAllActions() const;

private:
    std::map<GameAction, sf::Keyboard::Key> _keybinds;
    std::map<GameAction, std::string> _actionNames;
    
    void initializeDefaults();
    void initializeActionNames();
};

} // namespace CLIENT