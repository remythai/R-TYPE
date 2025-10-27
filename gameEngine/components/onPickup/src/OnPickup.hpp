#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct OnPickup : public Component<OnPickup>
{
    int hpBonus;
    int hpMaxBonus;
    int dmgBonus;
    float cooldownBonus;
    float scoreMultiplierBonus;
    float duration;
    OnPickup(
        int val_hpBonus = 0, int val_hpMaxBonus = 0, int val_dmgBonus = 0,
        float val_cooldownBonus = 0.0f, float val_scoreMultiplierBonus = 0.0f,
        float val_duration = 0.0f)
        : hpBonus(val_hpBonus),
          hpMaxBonus(val_hpMaxBonus),
          dmgBonus(val_dmgBonus),
          cooldownBonus(val_cooldownBonus),
          scoreMultiplierBonus(val_scoreMultiplierBonus),
          duration(val_duration)
    {
    }

    static constexpr const char* Name = "OnPickup";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
