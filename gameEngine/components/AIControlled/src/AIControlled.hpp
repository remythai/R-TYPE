#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct AIControlled  : public Component<AIControlled>
{
    AIControlled() {}

    static constexpr const char* Name = "AIControlled";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
