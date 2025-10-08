#pragma once

#include "../../../Registry.hpp"
#include "../../../System.hpp"

namespace GameEngine {
    class MovementSystem : public System<MovementSystem> {
    public:
        MovementSystem() {
            requireComponents<GameEngine::Position, GameEngine::Velocity>();
        }
        
        void onUpdate(Registry& registry, float dt) {
            updateCount++;
            
            registry.each<Position, Velocity>([dt](auto e, Position& pos, Velocity& vel) {
                pos.x += vel.x * dt;
                pos.y += vel.y * dt;
            });
        }
        
        int updateCount = 0;
    };
}
