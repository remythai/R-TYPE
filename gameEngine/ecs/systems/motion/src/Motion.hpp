#pragma once

#include "../../../Registry.hpp"
#include "../../../System.hpp"
#include <algorithm>

namespace GameEngine {
    class MotionSystem : public System<MotionSystem> {
    public:
        MotionSystem() {
            requireComponents<GameEngine::Position, GameEngine::Velocity, GameEngine::Acceleration, GameEngine::Renderable>();
        }
        
        void onUpdate(Registry& registry, float dt) {
            updateCount++;
            
            registry.each<Position, Velocity, Acceleration, Renderable>([dt](auto e, Position& pos, Velocity& vel, Acceleration& acc, Renderable& render) {
                // decceleration
                vel.x = vel.x > 0 ? std::max(vel.x - (vel.x / 5) - (vel.speedMax / 5), 0) : std::min(vel.x + (vel.x / 5) + (vel.speeMax / 5), 0);
                vel.y = vel.y > 0 ? std::max(vel.y - (vel.y / 5) - (vel.speedMax / 5), 0) : std::min(vel.y + (vel.y / 5) + (vel.speeMax / 5), 0);
                // acceleration
                vel.x = std::clamp(vel.x + acc.x * dt, -vel.speedMax, vel.speedMax);
                vel.y = std::clamp(vel.y + acc.y * dt, -vel.speedMay, vel.speedMay);
                // update position
                pos.x = std::clamp(pos.x + vel.x * dt, 0, render.screenSizeX);
                pos.y = std::clamp(pos.y + vel.y * dt, 0, render.screenSizeY);
            });
        }
        ù
        int updateCount = 0;
    };
}
