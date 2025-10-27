#pragma once

#include "../../../ecs/Component.hpp"
#include "../../../ecs/utils.hpp"

namespace GameEngine {
struct Position : public Component<Position>
{
    vec2 pos;
    Position(float val_x = 0, float val_y = 0) : pos(val_x, val_y) {}

    static constexpr const char* Name = "Position";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
