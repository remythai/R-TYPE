/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** AnimatedSprite.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

namespace CLIENT {

class AnimatedSprite {
public:
    AnimatedSprite();

    void setAnimation(sf::Texture *texture, 
                      const sf::Vector2u &frameSize,
                      int frameCount,
                      int row = 0,
                      float frameDuration = 0.1f);

    void update(float deltaTime);
    void draw(sf::RenderWindow& window) const;

    void play();
    void pause();
    void stop();
    void setLoop(bool loop);
    void setFrameDuration(float duration);

    void setFrame(int frame);
    int getCurrentFrame() const;
    int getFrameCount() const;

    sf::Sprite *getSprite();

    void setPosition(float x, float y);
    void setPosition(const sf::Vector2f& pos);
    void setScale(float x, float y);
    sf::Vector2f getPosition() const;

    sf::FloatRect getGlobalBounds() const;

private:
    std::unique_ptr<sf::Sprite> _sprite;
    sf::Texture* _texture;
    std::vector<sf::IntRect> _frames;
    sf::Vector2u _frameSize;

    int _currentFrame;
    int _frameCount;
    int _row;
    float _frameDuration;
    float _elapsedTime;
    bool _isPlaying;
    bool _loop;
};

} // namespace CLIENT
