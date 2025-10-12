#pragma once

namespace GameEngine {
struct Domain {
    float ax, ay, bx, by;
    Domain(float val_ax = 0, float val_ay = 0, float val_bx = 0, float val_by = 0) : ax(val_ax), ay(val_ay), bx(val_bx), by(val_by) {}
};
} // namespace GameEngine
