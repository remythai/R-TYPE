#pragma once

#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"
#include <algorithm>

namespace GameEngine {
    class DomainHandler : public System<DomainHandler> {
    public:
        DomainHandler() {
            requireComponents<GameEngine::Position, GameEngine::Domain>();
        }
        
        void onUpdate(Registry& registry, float dt) {
            updateCount++;
            
            registry.each<Position, Domain>([dt, &registry](auto e, Position& pos, Domain& domain) {
                if (pos.pos.x < domain.ax || pos.pos.x > domain.bx || pos.pos.y < domain.ay || pos.pos.y > domain.by)
                    registry.destroy(e);
            });
        }
        int updateCount = 0;
    };
}
