#pragma once
#include <cstddef>
#include <iostream>
#include "../../../ecs/utils.hpp"

namespace GameEngine {
struct Renderable {
    float screenSizeX, screenSizeY;
    std::string spriteSheetPath;
    vec2 rectPos;
    vec2 rectSize;
    std::size_t frameNumber;
    std::size_t currentFrame;
    float frameDuration;
    Renderable(float val_screenSizeX, float val_screenSizeY, std::string val_spriteSheetPath, vec2 val_rectPos, vec2 val_rectSize, std::size_t val_frameNumber, std::size_t val_currentFrame, float val_frameDuration) : screenSizeX(val_screenSizeX), screenSizeY(val_screenSizeY), spriteSheetPath(val_spriteSheetPath), rectPos(val_rectPos), rectSize(val_rectSize), frameNumber{val_frameNumber}, currentFrame(val_currentFrame), frameDuration(val_frameDuration) {}
};
} // namespace GameEngine
