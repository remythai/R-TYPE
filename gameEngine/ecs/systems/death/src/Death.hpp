#pragma once

#include "../../../Registry.hpp"
#include "../../../System.hpp"
#include "../../../components/health/src/Health.hpp"
#include <algorithm>

namespace GameEngine {
    class MotionSystem : public System<MotionSystem> {
    public:
        MotionSystem() {
            requireComponents<GameEngine::Health>();
        }
        
        void onUpdate(Registry& registry, float dt) {
            updateCount++;
            
            registry.each<Health>([dt, &registry](auto e, Health& health) {
                if (health.currentHp == 0) {
                    registry.destroy(e);
                }
            });
        }
        int updateCount = 0;
    };
}
