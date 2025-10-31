/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** ScoreDisplay.cpp
*/

#include "ScoreDisplay.hpp"
#include <iostream>

CLIENT::ScoreDisplay::ScoreDisplay(const std::string& fontPath)
    : _score(0), _fontLoaded(false), _scoreText(_font)
{
    if (!_font.openFromFile(fontPath)) {
        std::cerr << "[ScoreDisplay] Failed to load font: " << fontPath << std::endl;
        if (!_font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
            std::cerr << "[ScoreDisplay] Failed to load fallback font" << std::endl;
            return;
        }
    }
    
    _fontLoaded = true;
    _scoreText.setCharacterSize(24);
    _scoreText.setFillColor(sf::Color::White);
    _scoreText.setPosition(sf::Vector2f(10.f, 10.f));
    
    _scoreText.setOutlineColor(sf::Color::Black);
    _scoreText.setOutlineThickness(2.f);
    
    updateText();
}

void CLIENT::ScoreDisplay::setScore(int score)
{
    if (_score != score) {
        _score = score;
        updateText();
    }
}

void CLIENT::ScoreDisplay::updateText()
{
    _scoreText.setString("Score: " + std::to_string(_score));
}

void CLIENT::ScoreDisplay::render(sf::RenderTarget& target)
{
    if (_fontLoaded) {
        target.draw(_scoreText);
    }
}

void CLIENT::ScoreDisplay::setPosition(float x, float y)
{
    _scoreText.setPosition(sf::Vector2f(x, y));
}

void CLIENT::ScoreDisplay::setCharacterSize(unsigned int size)
{
    _scoreText.setCharacterSize(size);
}

void CLIENT::ScoreDisplay::setColor(const sf::Color& color)
{
    _scoreText.setFillColor(color);
}
