#pragma once
#include <string>

#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct FireRate : public Component<FireRate>
{
    float fireRate;
    float time;

    FireRate(float val_fireRate = 0.50F, float val_time = 0.0F)
        : fireRate(val_fireRate), time(val_time)
    {
    }

    static constexpr const char* Name = "FireRate";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
