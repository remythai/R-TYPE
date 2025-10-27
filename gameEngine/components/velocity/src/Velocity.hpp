#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {

struct Velocity : public Component<Velocity>
{
    float x;
    float y;
    float speedMax;

    Velocity(float val_speedMax = 10.0f, float val_x = 0, float val_y = 0)
        : x(val_x), y(val_y), speedMax(val_speedMax)
    {
    }

    static constexpr const char* Name = "Velocity";
    static constexpr const char* Version = "1.0.0";
};

}  // namespace GameEngine
