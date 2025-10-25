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

CLIENT::Window::Window(
    const std::string &title, unsigned int width, unsigned int height)
    : _window(sf::VideoMode(sf::Vector2u{width, height}), title),
      _deltaTime(0.0f)
{
    _window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(_window)) {
        std::cerr << "Failed to initialize ImGui-SFML" << std::endl;
    }
}

CLIENT::Window::~Window()
{
    ImGui::SFML::Shutdown();
}

bool CLIENT::Window::isOpen() const
{
    return _window.isOpen();
}

void CLIENT::Window::pollEvents()
{
    _deltaTime = _clock.restart().asSeconds();

    _pendingActions.clear();

    while (auto eventOpt = _window.pollEvent()) {
        const auto &event = *eventOpt;

        ImGui::SFML::ProcessEvent(_window, event);

        if (event.is<sf::Event::Closed>())
            _window.close();

        if (event.is<sf::Event::KeyPressed>()) {
            if (const auto *keyEvent = event.getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Q) {
                    _window.close();
                } else if (keyEvent->code == sf::Keyboard::Key::Space) {
                    std::cout << "Shoot\n";
                    _pendingActions.push_back("SHOOT");
                }
            }
        }
    }

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

    ImGui::SFML::Update(_window, sf::seconds(_deltaTime));
}

void CLIENT::Window::clear()
{
    _window.clear(sf::Color::Black);
}

void CLIENT::Window::display()
{
    ImGui::SFML::Render(_window);
    _window.display();
}

sf::RenderWindow &CLIENT::Window::getWindow()
{
    return _window;
}

const std::vector<std::string> &CLIENT::Window::getPendingActions() const
{
    return _pendingActions;
}

float CLIENT::Window::getDeltaTime() const
{
    return _deltaTime;
}
