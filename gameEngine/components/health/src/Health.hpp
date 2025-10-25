#pragma once

namespace GameEngine {
struct Health
{
    int currentHp;
    int maxHp;
    Health(float val_currentHp = 0, float val_maxHp = 0)
        : currentHp(val_currentHp), maxHp(val_maxHp)
    {
    }
};
}  // namespace GameEngine
