#pragma once

#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"
#include "../../../components/health/src/Health.hpp"
#include <algorithm>

namespace GameEngine {
    /**
    * @class Death
    * @brief System responsible for removing entities with zero health.
    *
    * The `Death` iterates over all entities that have a `Health` component.
    * If an entity’s `currentHp` is zero, the entity is destroyed via the ECS `Registry`.
    * 
    * This ensures dead entities are removed from the simulation automatically.
    */
    class Death : public System<Death> {
    public:
        /**
        * @brief Constructs the Health-cleanup system and declares its required component.
        *
        * Requires:
        * - `GameEngine::Health`
        */
        Death() {
            requireComponents<GameEngine::Health>();
        }
        
        /**
        * @brief Updates the system, removing dead entities.
        *
        * For each entity with a `Health` component:
        * - Checks if `currentHp` is zero.
        * - If so, destroys the entity in the registry.
        *
        * @param registry The ECS registry managing entities and components.
        * @param dt Delta time since the last update (unused here but kept for consistency with other systems).
        */
        void onUpdate(Registry& registry, float dt) {
            updateCount++;

            registry.each<Health>(
                [dt, &registry](auto e, Health& health) {
                    if (health.currentHp == 0) {
                        registry.destroy(e);
                    }
                }
            );
        }

        /// @brief Tracks how many times this system has run (useful for debugging or profiling).
        int updateCount = 0;
    };
} // namespace GameEngine
