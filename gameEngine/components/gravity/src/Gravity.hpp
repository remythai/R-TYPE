#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct Gravity : public Component<Gravity>
{
    float force;
    Gravity(float val_force = 0) : force(val_force) {}

    static constexpr const char* Name = "Gravity";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
