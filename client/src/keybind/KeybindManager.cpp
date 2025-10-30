/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** KeybindManager.cpp
*/

#include "KeybindManager.hpp"

#include <fstream>
#include <iostream>

/**
 * @brief Constructs a KeybindManager.
 *
 * @details
 * Initializes action names and default keybinds.
 */
CLIENT::KeybindManager::KeybindManager()
{
    initializeActionNames();
    initializeDefaults();
}

/**
 * @brief Initializes human-readable names for all game actions.
 */
void CLIENT::KeybindManager::initializeActionNames()
{
    _actionNames[GameAction::MOVE_UP] = "Move Up";
    _actionNames[GameAction::MOVE_DOWN] = "Move Down";
    _actionNames[GameAction::MOVE_LEFT] = "Move Left";
    _actionNames[GameAction::MOVE_RIGHT] = "Move Right";
    _actionNames[GameAction::SHOOT] = "Shoot";
    _actionNames[GameAction::OPEN_KEYBIND_MENU] = "Keybind Menu";
}

/**
 * @brief Initializes the default keybinds for all actions.
 */
void CLIENT::KeybindManager::initializeDefaults()
{
    _keybinds[GameAction::MOVE_UP] = sf::Keyboard::Key::Up;
    _keybinds[GameAction::MOVE_DOWN] = sf::Keyboard::Key::Down;
    _keybinds[GameAction::MOVE_LEFT] = sf::Keyboard::Key::Left;
    _keybinds[GameAction::MOVE_RIGHT] = sf::Keyboard::Key::Right;
    _keybinds[GameAction::SHOOT] = sf::Keyboard::Key::Space;
    _keybinds[GameAction::OPEN_KEYBIND_MENU] = sf::Keyboard::Key::K;
}

/**
 * @brief Sets a keybind for a specific game action.
 *
 * @param action The game action to bind.
 * @param key The SFML key to bind to the action.
 */
void CLIENT::KeybindManager::setKeybind(
    GameAction action, sf::Keyboard::Key key)
{
    _keybinds[action] = key;
}

/**
 * @brief Retrieves the keybind for a specific game action.
 *
 * @param action The game action to query.
 * @return The SFML key bound to the action, or Key::Unknown if not found.
 */
sf::Keyboard::Key CLIENT::KeybindManager::getKeybind(GameAction action) const
{
    auto it = _keybinds.find(action);
    if (it != _keybinds.end()) {
        return it->second;
    }
    return sf::Keyboard::Key::Unknown;
}

/**
 * @brief Retrieves the human-readable name of a game action.
 *
 * @param action The game action to query.
 * @return The name of the action, or "Unknown" if not found.
 */
std::string CLIENT::KeybindManager::getActionName(GameAction action) const
{
    auto it = _actionNames.find(action);
    if (it != _actionNames.end()) {
        return it->second;
    }
    return "Unknown";
}

/**
 * @brief Retrieves a human-readable name for a given SFML key.
 *
 * @param key The SFML key to query.
 * @return The string representation of the key, or "Unknown" if not recognized.
 */
std::string CLIENT::KeybindManager::getKeyName(sf::Keyboard::Key key) const
{
    static const std::map<sf::Keyboard::Key, std::string> keyNames = {
        {sf::Keyboard::Key::A, "A"},       {sf::Keyboard::Key::B, "B"},
        {sf::Keyboard::Key::C, "C"},       {sf::Keyboard::Key::D, "D"},
        {sf::Keyboard::Key::E, "E"},       {sf::Keyboard::Key::F, "F"},
        {sf::Keyboard::Key::G, "G"},       {sf::Keyboard::Key::H, "H"},
        {sf::Keyboard::Key::I, "I"},       {sf::Keyboard::Key::J, "J"},
        {sf::Keyboard::Key::K, "K"},       {sf::Keyboard::Key::L, "L"},
        {sf::Keyboard::Key::M, "M"},       {sf::Keyboard::Key::N, "N"},
        {sf::Keyboard::Key::O, "O"},       {sf::Keyboard::Key::P, "P"},
        {sf::Keyboard::Key::Q, "Q"},       {sf::Keyboard::Key::R, "R"},
        {sf::Keyboard::Key::S, "S"},       {sf::Keyboard::Key::T, "T"},
        {sf::Keyboard::Key::U, "U"},       {sf::Keyboard::Key::V, "V"},
        {sf::Keyboard::Key::W, "W"},       {sf::Keyboard::Key::X, "X"},
        {sf::Keyboard::Key::Y, "Y"},       {sf::Keyboard::Key::Z, "Z"},
        {sf::Keyboard::Key::Num0, "0"},    {sf::Keyboard::Key::Num1, "1"},
        {sf::Keyboard::Key::Num2, "2"},    {sf::Keyboard::Key::Num3, "3"},
        {sf::Keyboard::Key::Num4, "4"},    {sf::Keyboard::Key::Num5, "5"},
        {sf::Keyboard::Key::Num6, "6"},    {sf::Keyboard::Key::Num7, "7"},
        {sf::Keyboard::Key::Num8, "8"},    {sf::Keyboard::Key::Num9, "9"},
        {sf::Keyboard::Key::Space, "Space"}, {sf::Keyboard::Key::Enter, "Enter"},
        {sf::Keyboard::Key::Escape, "Escape"}, {sf::Keyboard::Key::LShift, "LShift"},
        {sf::Keyboard::Key::RShift, "RShift"}, {sf::Keyboard::Key::LControl, "LCtrl"},
        {sf::Keyboard::Key::RControl, "RCtrl"}, {sf::Keyboard::Key::LAlt, "LAlt"},
        {sf::Keyboard::Key::RAlt, "RAlt"}, {sf::Keyboard::Key::Up, "Up"},
        {sf::Keyboard::Key::Down, "Down"}, {sf::Keyboard::Key::Left, "Left"},
        {sf::Keyboard::Key::Right, "Right"}, {sf::Keyboard::Key::Tab, "Tab"},
        {sf::Keyboard::Key::Backspace, "Backspace"}
    };

    auto it = keyNames.find(key);
    if (it != keyNames.end()) {
        return it->second;
    }
    return "Unknown";
}

/**
 * @brief Checks if a given game action is currently pressed.
 *
 * @param action The game action to check.
 * @return True if the key for this action is pressed, false otherwise.
 */
bool CLIENT::KeybindManager::isActionPressed(GameAction action) const
{
    sf::Keyboard::Key key = getKeybind(action);
    return sf::Keyboard::isKeyPressed(key);
}

/**
 * @brief Returns a list of all available game actions.
 *
 * @return Vector of all GameAction values.
 */
std::vector<CLIENT::GameAction> CLIENT::KeybindManager::getAllActions() const
{
    return {GameAction::MOVE_UP,   GameAction::MOVE_DOWN,
            GameAction::MOVE_LEFT, GameAction::MOVE_RIGHT,
            GameAction::SHOOT,     GameAction::OPEN_KEYBIND_MENU};
}

/**
 * @brief Saves current keybinds to a file.
 *
 * @param filename Path to the file to save keybinds.
 *
 * @details
 * Each line in the file contains the integer values of the action and key.
 */
void CLIENT::KeybindManager::saveToFile(const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to save keybinds to " << filename << std::endl;
        return;
    }

    for (const auto& [action, key] : _keybinds) {
        file << static_cast<int>(action) << " " << static_cast<int>(key)
             << "\n";
    }

    file.close();
    std::cout << "Keybinds saved to " << filename << std::endl;
}

/**
 * @brief Loads keybinds from a file.
 *
 * @param filename Path to the file to load keybinds from.
 *
 * @details
 * If the file does not exist, the default keybinds are used.
 */
void CLIENT::KeybindManager::loadFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "No keybind file found, using defaults" << std::endl;
        return;
    }

    int actionInt;
    int keyInt;
    while (file >> actionInt >> keyInt) {
        GameAction action = static_cast<GameAction>(actionInt);
        sf::Keyboard::Key key = static_cast<sf::Keyboard::Key>(keyInt);
        _keybinds[action] = key;
    }

    file.close();
    std::cout << "Keybinds loaded from " << filename << std::endl;
}

/**
 * @brief Resets all keybinds to their default values.
 */
void CLIENT::KeybindManager::resetToDefaults()
{
    initializeDefaults();
    std::cout << "Keybinds reset to defaults" << std::endl;
}

