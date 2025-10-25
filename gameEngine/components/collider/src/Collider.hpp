#pragma once

#include <bitset>
#include <variant>

#include "../../../ecs/utils.hpp"
namespace GameEngine {
struct HitBox
{};
struct Collider
{
    vec2 originTranslation;
    std::bitset<8> entitySelector;
    std::bitset<8> entityDiff;
    vec2 size;
    Collider(
        vec2 val_translation, std::bitset<8> val_selector,
        std::bitset<8> val_diff, vec2 val_size)
        : originTranslation(val_translation),
          entitySelector(val_selector),
          entityDiff(val_diff),
          size(val_size)
    {
    }
};
}  // namespace GameEngine
