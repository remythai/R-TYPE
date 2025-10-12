#pragma once

#include "../../../utils.hpp"

namespace GameEngine {
struct Position {
    vec2 pos;
    Position(float val_x = 0, float val_y = 0) : pos(val_x, val_y) {}
};
} // namespace GameEngine
