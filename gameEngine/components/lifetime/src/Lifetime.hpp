#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct Lifetime : public Component<Lifetime>
{
    float time;
    Lifetime(float val_time = 0) : time(val_time) {}

    static constexpr const char* Name = "Lifetime";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
