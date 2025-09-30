/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** AnimatedSprite.cpp
*/

#include "AnimatedSprite.hpp"

namespace CLIENT {

AnimatedSprite::AnimatedSprite()
    : _sprite(nullptr)
    , _texture(nullptr)
    , _currentFrame(0)
    , _frameDuration(0.1f)
    , _elapsedTime(0.0f)
    , _isPlaying(true)
    , _loop(true)
    , _frameCount(0)
    , _row(0)
{}

void AnimatedSprite::setAnimation(sf::Texture *texture, 
                                  const sf::Vector2u &frameSize,
                                  int frameCount,
                                  int row,
                                  float frameDuration)
{
    _texture = texture;
    _frameSize = frameSize;
    _frameCount = frameCount;
    _row = row;
    _frameDuration = frameDuration;
    _currentFrame = 0;
    _elapsedTime = 0.0f;

    _frames.clear();
    for (int i = 0; i < frameCount; ++i) {
        sf::IntRect rect(
            sf::Vector2i(i * frameSize.x, row * frameSize.y),
            sf::Vector2i(frameSize.x, frameSize.y)
        );
        _frames.push_back(rect);
    }

    if (_texture) {
        _sprite = std::make_unique<sf::Sprite>(*_texture, _frames[0]);
    }
}

void AnimatedSprite::update(float deltaTime) {
    if (!_isPlaying || _frames.empty() || !_sprite)
        return;

    _elapsedTime += deltaTime;

    if (_elapsedTime >= _frameDuration) {
        _elapsedTime = 0.0f;
        _currentFrame++;

        if (_currentFrame >= _frameCount) {
            if (_loop)
                _currentFrame = 0;
            else {
                _currentFrame = _frameCount - 1;
                _isPlaying = false;
            }
        }

        _sprite->setTextureRect(_frames[_currentFrame]);
    }
}

void AnimatedSprite::draw(sf::RenderWindow &window) const {
    if (_sprite)
        window.draw(*_sprite);
}

void AnimatedSprite::play() {
    _isPlaying = true;
}

void AnimatedSprite::pause() {
    _isPlaying = false;
}

void AnimatedSprite::stop() { 
    _isPlaying = false; 
    _currentFrame = 0;
    if (!_frames.empty() && _sprite)
        _sprite->setTextureRect(_frames[0]);
}

void AnimatedSprite::setLoop(bool loop) {
    _loop = loop;
}

void AnimatedSprite::setFrameDuration(float duration) {
    _frameDuration = duration;
}

void AnimatedSprite::setFrame(int frame) {
    if (frame >= 0 && frame < _frameCount && _sprite && !_frames.empty()) {
        _currentFrame = frame;
        _sprite->setTextureRect(_frames[_currentFrame]);
    }
}

int AnimatedSprite::getCurrentFrame() const {
    return _currentFrame;
}

int AnimatedSprite::getFrameCount() const {
    return _frameCount;
}

sf::Sprite *AnimatedSprite::getSprite() {
    return _sprite.get();
}

void AnimatedSprite::setPosition(float x, float y) { 
    if (_sprite)
        _sprite->setPosition(sf::Vector2f(x, y)); 
}

void AnimatedSprite::setScale(float x, float y) { 
    if (_sprite) 
        _sprite->setScale(sf::Vector2f(x, y)); 
}

sf::Vector2f AnimatedSprite::getPosition() const { 
    return _sprite ? _sprite->getPosition() : sf::Vector2f(0, 0); 
}

} // namespace CLIENT
