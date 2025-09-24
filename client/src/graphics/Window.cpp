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
: _window(sf::VideoMode(sf::Vector2u{width, height}), title)
{
    _window.setFramerateLimit(60);
}

Window::~Window() = default;

bool Window::isOpen() const {
    return _window.isOpen();
}

void Window::pollEvents() {
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
                        break;
                    case sf::Keyboard::Key::Down:
                        std::cout << "Down\n";
                        break;
                    case sf::Keyboard::Key::Left:
                        std::cout << "Left\n";
                        break;
                    case sf::Keyboard::Key::Right:
                        std::cout << "Right\n";
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

sf::RenderWindow &Window::getHandle() {
    return _window;
}

} // namespace CLIENT