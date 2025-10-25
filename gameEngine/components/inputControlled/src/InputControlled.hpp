#pragma once

#include <string>
#include <vector>
namespace GameEngine {
struct InputControlled
{
    std::vector<int> inputs;
    bool firstInput;
    InputControlled() : firstInput(false) {}
};
}  // namespace GameEngine
