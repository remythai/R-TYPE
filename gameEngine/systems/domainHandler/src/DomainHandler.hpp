#pragma once

#include "../../../Registry.hpp"
#include "../../../System.hpp"
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
                if (pos.x < domain.ax || pos.x > domain.bx || pos.y < domain.ay || pos.y > domain.by)
                    registry.destroy(e);
            });
        }
        int updateCount = 0;
    };
}
