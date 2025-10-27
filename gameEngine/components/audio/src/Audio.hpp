#pragma once
#include <string>

#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct Audio : public Component<Audio>
{
    std::string soundName;
    float volume;
    bool loop;

    Audio(
        const std::string& val_soundName = "", float val_volume = 1.0f,
        bool val_loop = false)
        : soundName(val_soundName), volume(val_volume), loop(val_loop)
    {
    }

    static constexpr const char* Name = "Audio";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
