/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** AnimatedSprite.cpp
*/

#include "AnimatedSprite.hpp"

CLIENT::AnimatedSprite::AnimatedSprite()
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

void CLIENT::AnimatedSprite::setAnimation(sf::Texture *texture, 
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

void CLIENT::AnimatedSprite::update(float deltaTime) {
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

void CLIENT::AnimatedSprite::draw(sf::RenderWindow &window) const {
    if (_sprite)
        window.draw(*_sprite);
}

void CLIENT::AnimatedSprite::play() {
    _isPlaying = true;
}

void CLIENT::AnimatedSprite::pause() {
    _isPlaying = false;
}

void CLIENT::AnimatedSprite::stop() { 
    _isPlaying = false; 
    _currentFrame = 0;
    if (!_frames.empty() && _sprite)
        _sprite->setTextureRect(_frames[0]);
}

void CLIENT::AnimatedSprite::setLoop(bool loop) {
    _loop = loop;
}

void CLIENT::AnimatedSprite::setFrameDuration(float duration) {
    _frameDuration = duration;
}

void CLIENT::AnimatedSprite::setFrame(int frame) {
    if (frame >= 0 && frame < _frameCount && _sprite && !_frames.empty()) {
        _currentFrame = frame;
        _sprite->setTextureRect(_frames[_currentFrame]);
    }
}

int CLIENT::AnimatedSprite::getCurrentFrame() const {
    return _currentFrame;
}

int CLIENT::AnimatedSprite::getFrameCount() const {
    return _frameCount;
}

sf::Sprite *CLIENT::AnimatedSprite::getSprite() {
    return _sprite.get();
}

void CLIENT::AnimatedSprite::setPosition(float x, float y) { 
    if (_sprite)
        _sprite->setPosition(sf::Vector2f(x, y)); 
}

void CLIENT::AnimatedSprite::setScale(float x, float y) { 
    if (_sprite) 
        _sprite->setScale(sf::Vector2f(x, y)); 
}

sf::Vector2f CLIENT::AnimatedSprite::getPosition() const { 
    return _sprite ? _sprite->getPosition() : sf::Vector2f(0, 0); 
}

void CLIENT::AnimatedSprite::setPosition(const sf::Vector2f& pos)
{
    if (_sprite)
        _sprite->setPosition(pos);
}

sf::FloatRect CLIENT::AnimatedSprite::getGlobalBounds() const
{
    if (_sprite)
        return _sprite->getGlobalBounds();
    return sf::FloatRect();
}
