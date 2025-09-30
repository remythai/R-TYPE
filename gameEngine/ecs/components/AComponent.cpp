#include "AComponent.hpp"
#include <string>

GameEngine::AComponent::AComponent(std::string tag) : _tag(tag) {}

std::string GameEngine::AComponent::get_tag() const {
    return this->_tag;
}
