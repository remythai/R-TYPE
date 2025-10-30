/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Window.cpp
*/

#include "Window.hpp"

#include <imgui-SFML.h>
#include <imgui.h>

#include <iostream>

/**
 * @brief Constructs a Window with a given title, width, and height.
 *
 * @param title The window title.
 * @param width Width of the window in pixels.
 * @param height Height of the window in pixels.
 *
 * @details
 * Initializes the SFML RenderWindow and sets a framerate limit of 60 FPS.
 * Also initializes ImGui-SFML for GUI rendering. If ImGui initialization fails,
 * an error message is printed.
 */
CLIENT::Window::Window(
    const std::string &title, unsigned int width, unsigned int height)
    : _window(sf::VideoMode(sf::Vector2u{width, height}), title),
      _deltaTime(0.0f),
      _keybindManager(nullptr),
      _keybindMenu(nullptr)
{
    _window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(_window)) {
        std::cerr << "Failed to initialize ImGui-SFML" << std::endl;
    }
}

/**
 * @brief Destructor for the Window class.
 *
 * @details
 * Shuts down ImGui-SFML and releases any resources associated with the window.
 */
CLIENT::Window::~Window()
{
    ImGui::SFML::Shutdown();
}

/**
 * @brief Checks if the window is open.
 *
 * @return True if the window is open, false otherwise.
 */
bool CLIENT::Window::isOpen() const
{
    return _window.isOpen();
}

/**
 * @brief Polls events from the window and processes input.
 *
 * @details
 * Handles window close events, key input for player actions, and forwards
 * events to ImGui and the keybind menu if it is open. Updates delta time
 * for the current frame.
 */
void CLIENT::Window::pollEvents()
{
    if (!_window.isOpen()) {
        return;
    }

    _deltaTime = _clock.restart().asSeconds();

    _pendingActions.clear();

    while (auto eventOpt = _window.pollEvent()) {
        const auto &event = *eventOpt;

        if (event.is<sf::Event::Closed>()) {
            _window.close();
            return;
        }

        if (_keybindMenu && _keybindMenu->isOpen()) {
            _keybindMenu->handleEvent(event);
            ImGui::SFML::ProcessEvent(_window, event);
            continue;
        }

        ImGui::SFML::ProcessEvent(_window, event);

        if (event.is<sf::Event::KeyPressed>()) {
            if (const auto *keyEvent = event.getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Q) {
                    _window.close();
                    return;
                }
                if (_keybindManager) {
                    if (keyEvent->code ==
                        _keybindManager->getKeybind(GameAction::SHOOT)) {
                        std::cout << "Shoot\n";
                        _pendingActions.emplace_back("SHOOT");
                    }
                    if (keyEvent->code == _keybindManager->getKeybind(
                                              GameAction::OPEN_KEYBIND_MENU)) {
                        if (_keybindMenu) {
                            _keybindMenu->open();
                        }
                    }
                }
            }
        }
    }

    if (!_window.isOpen()) {
        return;
    }

    if (!_keybindMenu || !_keybindMenu->isOpen()) {
        if (_keybindManager) {
            if (_keybindManager->isActionPressed(GameAction::MOVE_UP)) {
                std::cout << "Up\n";
                _pendingActions.push_back("MOVE_UP");
            }
            if (_keybindManager->isActionPressed(GameAction::MOVE_DOWN)) {
                std::cout << "Down\n";
                _pendingActions.push_back("MOVE_DOWN");
            }
            if (_keybindManager->isActionPressed(GameAction::MOVE_LEFT)) {
                std::cout << "Left\n";
                _pendingActions.push_back("MOVE_LEFT");
            }
            if (_keybindManager->isActionPressed(GameAction::MOVE_RIGHT)) {
                std::cout << "Right\n";
                _pendingActions.push_back("MOVE_RIGHT");
            }
        } else {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                std::cout << "Up\n";
                _pendingActions.push_back("MOVE_UP");
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
                std::cout << "Down\n";
                _pendingActions.push_back("MOVE_DOWN");
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                std::cout << "Left\n";
                _pendingActions.push_back("MOVE_LEFT");
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                std::cout << "Right\n";
                _pendingActions.push_back("MOVE_RIGHT");
            }
        }
    }

    if (_window.isOpen()) {
        ImGui::SFML::Update(_window, sf::seconds(_deltaTime));
    }
}

/**
 * @brief Clears the window with a black color.
 */
void CLIENT::Window::clear()
{
    if (_window.isOpen()) {
        _window.clear(sf::Color::Black);
    }
}

/**
 * @brief Displays the contents of the window.
 *
 * @details
 * Renders ImGui contents and swaps the window buffers.
 */
void CLIENT::Window::display()
{
    if (_window.isOpen()) {
        ImGui::SFML::Render(_window);
        _window.display();
    }
}

/**
 * @brief Retrieves the underlying SFML RenderWindow.
 *
 * @return Reference to the sf::RenderWindow.
 */
sf::RenderWindow &CLIENT::Window::getWindow()
{
    return _window;
}

/**
 * @brief Retrieves the list of pending actions for this frame.
 *
 * @return Reference to the vector of pending actions.
 */
const std::vector<std::string> &CLIENT::Window::getPendingActions() const
{
    return _pendingActions;
}

/**
 * @brief Retrieves the delta time of the current frame.
 *
 * @return Delta time in seconds.
 */
float CLIENT::Window::getDeltaTime() const
{
    return _deltaTime;
}

/**
 * @brief Sets the keybind manager and menu components for the window.
 *
 * @param manager Pointer to the KeybindManager.
 * @param menu Pointer to the KeybindMenu.
 *
 * @details
 * These components are used to handle input events and display the keybind menu.
 */
void CLIENT::Window::setKeybindComponents(
    KeybindManager *manager, KeybindMenu *menu)
{
    _keybindManager = manager;
    _keybindMenu = menu;
}
