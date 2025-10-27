#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct Damage : public Component<Damage>
{
    int dmg;
    Damage(int val_dmg = 0) : dmg(val_dmg) {}

    static constexpr const char* Name = "Damage";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
