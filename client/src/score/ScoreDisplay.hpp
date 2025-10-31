/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** ScoreDisplay.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <string>

namespace CLIENT {

/**
 * @brief Handles the display of the game score on screen
 */
class ScoreDisplay {
public:
    /**
     * @brief Construct a new Score Display object
     * @param fontPath Path to the font file to use
     */
    explicit ScoreDisplay(const std::string& fontPath = "assets/fonts/arial.ttf");

    /**
     * @brief Update the displayed score value
     * @param score The new score value
     */
    void setScore(int score);

    /**
     * @brief Get the current score value
     * @return int Current score
     */
    int getScore() const { return _score; }

    /**
     * @brief Render the score display
     * @param target The render target to draw on
     */
    void render(sf::RenderTarget& target);

    /**
     * @brief Set the position of the score display
     * @param x X coordinate
     * @param y Y coordinate
     */
    void setPosition(float x, float y);

    /**
     * @brief Set the character size of the text
     * @param size Character size in pixels
     */
    void setCharacterSize(unsigned int size);

    /**
     * @brief Set the color of the score text
     * @param color SFML Color
     */
    void setColor(const sf::Color& color);

private:
    sf::Font _font;
    sf::Text _scoreText;
    int _score;
    bool _fontLoaded;

    void updateText();
    };
}