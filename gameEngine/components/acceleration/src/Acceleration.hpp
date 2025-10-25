#pragma once

namespace GameEngine {
struct Acceleration
{
    float x, y;
    Acceleration(float val_x = 0, float val_y = 0) : x(val_x), y(val_y) {}
};
}  // namespace GameEngine
