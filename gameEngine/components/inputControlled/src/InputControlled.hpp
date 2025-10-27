#pragma once

#include <string>
#include <vector>

#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct InputControlled : public Component<InputControlled>
{
    std::vector<int> inputs;
    bool firstInput;
    InputControlled() : firstInput(false) {}

    static constexpr const char* Name = "InputControlled";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
