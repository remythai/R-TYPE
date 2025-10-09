#pragma once

namespace GameEngine {
struct Renderable {
    float screenSizeX, screenSizeY;
    Renderable(float val_screenSizeX, float val_screenSizeY) : screenSizeX(val_screenSizeX), screenSizeY(val_screenSizeY) {}
};
} // namespace GameEngine
