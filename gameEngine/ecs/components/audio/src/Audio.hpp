#pragma once
#include <string>

namespace GameEngine {
struct Audio {
    std::string soundName;
    float volume;
    bool loop;

    Audio(const std::string& val_soundName = "", float val_volume = 1.0f, bool val_loop = false) : soundName(val_soundName), volume(val_volume), loop(val_loop) {}
};
}
