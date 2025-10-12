#pragma once

#include <bitset>
#include <variant>
#include "../../../ecs/utils.hpp"
namespace GameEngine {
struct HitBox {
};
struct Collider {
    vec2 originTranslation;
    std::bitset<8> entitySelector;
    // std::variant<vec2, float> size; // vec2 -> box size, float -> radius
    vec2 size;
    Collider(vec2 val_translation, std::bitset<8> val_selector, vec2 val_size) : originTranslation(val_translation), entitySelector(val_selector), size(val_size) {}
};
} // namespace GameEngine
