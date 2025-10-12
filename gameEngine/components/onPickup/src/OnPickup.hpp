#pragma once

namespace GameEngine {
struct OnPickup {
    int hpBonus;
    int hpMaxBonus;
    int dmgBonus;
    float cooldownBonus;
    float scoreMultiplierBonus;
    float duration;
    OnPickup(int val_hpBonus, int val_hpMaxBonus, int val_dmgBonus, float val_cooldownBonus, float val_scoreMultiplierBonus, float val_duration) : hpBonus(val_hpBonus), hpMaxBonus(val_hpMaxBonus), dmgBonus(val_dmgBonus), cooldownBonus(val_cooldownBonus), scoreMultiplierBonus(val_scoreMultiplierBonus), duration(val_duration) {}
};
} // namespace GameEngine
