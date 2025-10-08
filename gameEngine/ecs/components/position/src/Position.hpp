#pragma once

namespace GameEngine {
struct Position {
    float x, y;
    Position(float val_x = 0, float val_y = 0) : x(val_x), y(val_y) {}
};
} // namespace GameEngine
