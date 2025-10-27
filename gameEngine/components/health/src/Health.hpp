#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct Health : public Component<Health>
{
    int currentHp;
    int maxHp;
    Health(float val_currentHp = 0, float val_maxHp = 0)
        : currentHp(val_currentHp), maxHp(val_maxHp)
    {
    }

    static constexpr const char* Name = "Health";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
