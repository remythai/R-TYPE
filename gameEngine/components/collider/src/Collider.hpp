#pragma once

#include <bitset>
#include <variant>

#include "../../../ecs/utils.hpp"
#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct HitBox
{};
struct Collider : public Component<Collider>
{
    vec2 originTranslation;
    std::bitset<8> entitySelector;
    std::bitset<8> entityDiff;
    vec2 size;
    Collider(
        vec2 val_translation = vec2(0, 0), std::bitset<8> val_selector = std::bitset<8>(0),
        std::bitset<8> val_diff = std::bitset<8>(0), vec2 val_size = vec2(0, 0))
        : originTranslation(val_translation),
          entitySelector(val_selector),
          entityDiff(val_diff),
          size(val_size)
    {
    }

    static constexpr const char* Name = "Collider";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
