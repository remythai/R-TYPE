#pragma once

#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"
#include "../../../components/position/src/Position.hpp"
#include "../../../components/velocity/src/Velocity.hpp"
#include "../../../components/acceleration/src/Acceleration.hpp"
#include "../../../components/renderable/src/Renderable.hpp"
#include <algorithm>

namespace GameEngine {
    class Motion : public System<Motion> {
    public:
        Motion() {
            requireComponents<GameEngine::Position, GameEngine::Velocity, GameEngine::Acceleration, GameEngine::Renderable>();
        }
        
        void onUpdate(Registry& registry, float dt) {
            updateCount++;
            
            registry.each<Position, Velocity, Acceleration, Renderable>([dt](auto e, Position& pos, Velocity& vel, Acceleration& acc, Renderable& render) {
                // decceleration
                vel.x = vel.x > 0 ? std::max(float(vel.x - (vel.x * 0.75)), float(0)) : std::min(float(vel.x - (vel.x * 0.75)), float(0));
                vel.y = vel.y > 0 ? std::max(float(vel.y - (vel.y * 0.75)), float(0)) : std::min(float(vel.y - (vel.y * 0.75)), float(0));
                // acceleration
                vel.x = std::clamp(vel.x + acc.x, -vel.speedMax, vel.speedMax);
                vel.y = std::clamp(vel.y + acc.y, -vel.speedMax, vel.speedMax);
                // update position
                pos.pos.x = std::clamp(pos.pos.x + vel.x, float(0), render.screenSizeX);
                pos.pos.y = std::clamp(pos.pos.y + vel.y, float(0), render.screenSizeY);
            });
        }
        int updateCount = 0;
    };
}
