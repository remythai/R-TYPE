#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct Acceleration : public Component<Acceleration>
{
    float x, y;
    Acceleration(float val_x = 0, float val_y = 0) : x(val_x), y(val_y) {}

    static constexpr const char* Name = "Acceleration";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
