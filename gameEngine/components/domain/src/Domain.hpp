#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
struct Domain : public Component<Domain>
{
    float ax, ay, bx, by;
    Domain(
        float val_ax = 0, float val_ay = 0, float val_bx = 0, float val_by = 0)
        : ax(val_ax), ay(val_ay), bx(val_bx), by(val_by)
    {
    }

    static constexpr const char* Name = "Domain";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
