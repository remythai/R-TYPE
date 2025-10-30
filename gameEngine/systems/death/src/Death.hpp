#pragma once

#include <algorithm>
#include <functional>

#include "../../../components/health/src/Health.hpp"
#include "../../../components/inputControlled/src/InputControlled.hpp"
#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"

namespace GameEngine {
/**
 * @class Death
 * @brief System that removes entities with zero health from the simulation.
 *
 * The Death system iterates through all entities that possess a Health
 * component and automatically destroys any entity whose `currentHp` reaches
 * zero. This ensures that dead entities are cleaned up from the ECS registry
 * automatically each frame.
 *
 * @details
 * This system is part of the damage and death handling pipeline:
 * 1. Other systems reduce entity health (e.g., DamageSystem)
 * 2. Death system checks for zero health each update
 * 3. Dead entities are removed from the simulation
 *
 * This approach decouples the death logic from damage dealing, allowing
 * flexible death handling without tight coupling.
 *
 * @note An entity is only destroyed if `currentHp == 0` (exact equality).
 *       This allows for potential "undead" mechanics where health might go
 * negative.
 *
 * @performance
 * - Time Complexity: O(n) where n is the number of entities with Health
 * components
 * - Space Complexity: O(1) - only iteration, no additional storage
 *
 * @see Health
 * @see Registry::each
 * @see Registry::destroy
 */
class Death : public System<Death>
{
   public:
    /**
     * @brief Constructs the Death system and registers its component
     * requirements.
     *
     * During construction, the system declares that it requires the Health
     * component. The ECS registry will only activate this system when at least
     * one entity with a Health component exists.
     *
     * @post System is configured to process entities with Health components.
     * @post The system is ready for scheduling within the Registry.
     */
    Death()
    {
        requireComponents<GameEngine::Health, InputControlled>();
    }

    /**
     * @brief Processes all entities with Health components and destroys those
     * with zero HP.
     *
     * Called every frame by the Registry. This method:
     * 1. Increments the update counter for profiling/debugging
     * 2. Iterates through all entities with a Health component
     * 3. Checks each entity's `currentHp` value
     * 4. Destroys any entity with exactly zero health
     *
     * @param registry Reference to the main ECS Registry for entity management.
     * @param dt Delta time since last update in seconds. Unused but provided
     *           for consistency with the System interface.
     *
     * @attention Destroying entities during iteration is safe because the ECS
     * system uses optimized pool iteration that doesn't invalidate cursors.
     *
     * @warning If an entity's health goes below zero (negative HP), it will NOT
     * be destroyed by this system. Ensure damage systems clamp health to 0.
     *
     * @example
     * ```cpp
     * Registry registry;
     * Death deathSystem;
     *
     * // If entity `e` has Health with currentHp = 0:
     * registry.update(0.016f); // Entity `e` will be destroyed
     * ```
     *
     * @see Registry::each
     * @see Registry::destroy
     * @see Health::currentHp
     */
    void onUpdate(Registry& registry, float dt)
    {
        updateCount++;

        registry.each<Health, InputControlled>([this, &registry](auto e, Health& health, InputControlled&) {
            if (health.currentHp == 0) {
                if (onPlayerDeath)
                    onPlayerDeath(e);
                registry.destroy(e);
            }
        });

        registry.each<Health>([dt, &registry](auto e, Health& health) {
            if (health.currentHp == 0) {
                registry.destroy(e);
            }
        });
    }

    /**
     * @brief Counter tracking system execution calls.
     *
     * Increments each time onUpdate() is called. Useful for:
     * - Performance profiling
     * - Unit testing (verifying the system runs)
     * - Debugging system execution order
     *
     * @remarks Value persists throughout the system's lifetime.
     *          Manually reset if needed for test isolation.
     */
    int updateCount = 0;
    std::function<void(EntityManager::Entity)> onPlayerDeath;
};
}  // namespace GameEngine
