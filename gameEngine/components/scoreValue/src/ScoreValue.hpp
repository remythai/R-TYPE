#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct ScoreValue : public Component<ScoreValue>
{
    int points;
    ScoreValue(int val_points = 0) : points(val_points) {}

    static constexpr const char* Name = "ScoreValue";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
