#pragma once
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

#include "../../../ecs/utils.hpp"
#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct Renderable : public Component<Renderable> {
    float screenSizeX, screenSizeY;
    std::string spriteSheetPath;
    vec2 currentRectPos;
    std::vector<vec2> rectPos;
    vec2 rectSize;
    int frameDuration;
    bool autoAnimate;

    Renderable() 
        : screenSizeX(0), 
          screenSizeY(0), 
          spriteSheetPath(""), 
          currentRectPos{0, 0},
          rectPos(),
          rectSize{0, 0}, 
          frameDuration(0), 
          autoAnimate(false) {}

    Renderable(float val_screenSizeX, float val_screenSizeY, std::string val_spriteSheetPath, 
               std::vector<vec2> val_rectPos, vec2 val_rectSize, int val_frameDuration, bool val_autoAnimate) 
        : screenSizeX(val_screenSizeX), 
          screenSizeY(val_screenSizeY), 
          spriteSheetPath(std::move(val_spriteSheetPath)), 
          rectPos(std::move(val_rectPos)), 
          rectSize(val_rectSize), 
          frameDuration(val_frameDuration), 
          autoAnimate(val_autoAnimate) {}

    static constexpr const char* Name = "Renderable";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
