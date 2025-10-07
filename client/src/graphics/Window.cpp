/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Window.cpp
*/

#include "Window.hpp"
#include <iostream>

namespace CLIENT {

Window::Window(const std::string &title, unsigned int width, unsigned int height)
: _window(sf::VideoMode(sf::Vector2u{width, height}), title), _deltaTime(0.0f)
{
    _window.setFramerateLimit(60);
}

Window::~Window() = default;

bool Window::isOpen() const {
    return _window.isOpen();
}

void Window::pollEvents() {
    _deltaTime = _clock.restart().asSeconds();
    
    _pendingActions.clear();
    
    while (auto eventOpt = _window.pollEvent()) {
        const auto &event = *eventOpt;

        if (event.is<sf::Event::Closed>())
            _window.close();

        if (event.is<sf::Event::KeyPressed>()) {
            if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
                switch (keyEvent->code) {
                    case sf::Keyboard::Key::Q:
                        _window.close();
                        break;
                    case sf::Keyboard::Key::Up:
                        std::cout << "Up\n";
                        _pendingActions.push_back("MOVE_UP");
                        break;
                    case sf::Keyboard::Key::Down:
                        std::cout << "Down\n";
                        _pendingActions.push_back("MOVE_DOWN");
                        break;
                    case sf::Keyboard::Key::Left:
                        std::cout << "Left\n";
                        _pendingActions.push_back("MOVE_LEFT");
                        break;
                    case sf::Keyboard::Key::Right:
                        std::cout << "Right\n";
                        _pendingActions.push_back("MOVE_RIGHT");
                        break;
                    case sf::Keyboard::Key::Space:
                        std::cout << "Shoot\n";
                        _pendingActions.push_back("SHOOT");
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void Window::clear() {
    _window.clear(sf::Color::Black);
}

void Window::display() {
    _window.display();
}

sf::RenderWindow &Window::getWindow() {
    return _window;
}

const std::vector<std::string> &Window::getPendingActions() const {
    return _pendingActions;
}

float Window::getDeltaTime() const {
    return _deltaTime;
}

} // namespace CLIENT