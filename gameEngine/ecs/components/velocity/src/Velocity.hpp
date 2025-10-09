#pragma once

namespace GameEngine {
struct Velocity {
    float x, y;
    float speedMax;
    Velocity(float val_speedMax, float val_x = 0, float val_y = 0) : x(val_x), y(val_y), speedMax(val_speedMax) {}
};
} // namespace GameEngine
