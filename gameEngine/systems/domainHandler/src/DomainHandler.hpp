#pragma once

#include <algorithm>

#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"

namespace GameEngine {
/**
 * @class DomainHandler
 * @brief System that manages entity domains and destroys entities outside their
 * boundaries.
 *
 * The DomainHandler is an ECS system that monitors entities with Position and
 * Domain components. It automatically destroys any entity that moves outside
 * the boundaries defined by its Domain component.
 *
 * This system is useful for:
 * - Removing projectiles that leave the game area
 * - Cleaning up entities that go out of bounds
 * - Enforcing level boundaries
 *
 * @details
 * The system iterates through all entities that have both Position and Domain
 * components. For each entity, it checks if the position (pos.x, pos.y) is
 * within the domain boundaries:
 * - **ax, ay**: Lower-left corner coordinates
 * - **bx, by**: Upper-right corner coordinates
 *
 * If an entity is outside these boundaries, it is automatically destroyed.
 *
 * @note This system requires entities to have both Position and Domain
 * components to be processed.
 * @see Position
 * @see Domain
 */
class DomainHandler : public System<DomainHandler>
{
   public:
    /**
     * @brief Constructs the DomainHandler system and sets up component
     * requirements.
     *
     * This constructor registers the Position and Domain components as
     * requirements for the system to be active and process entities.
     *
     * @post The system is configured to only process entities with Position and
     * Domain components.
     */
    DomainHandler()
    {
        requireComponents<GameEngine::Position, GameEngine::Domain>();
    }

    /**
     * @brief Updates the system, checking entity positions against their domain
     * boundaries.
     *
     * This method is called each frame by the Registry. It:
     * 1. Increments the internal update counter
     * 2. Iterates through all entities with Position and Domain components
     * 3. Checks if each entity's position is within its domain boundaries
     * 4. Destroys any entity found outside its boundaries
     *
     * @param registry Reference to the main ECS registry for entity and
     * component access.
     * @param dt Delta time since the last update in seconds.
     *
     * @details
     * An entity is considered outside its domain if:
     * - `pos.pos.x < domain.ax` (left of lower-left corner)
     * - `pos.pos.x > domain.bx` (right of upper-right corner)
     * - `pos.pos.y < domain.ay` (below lower-left corner)
     * - `pos.pos.y > domain.by` (above upper-right corner)
     *
     * @note Destroying entities during iteration is safe because the system
     * uses the smallest pool for optimization, which doesn't invalidate
     * iterators for subsequent entities.
     *
     * @see Registry::each
     * @see Registry::destroy
     */
    void onUpdate(Registry& registry, float dt)
    {
        updateCount++;

        registry.each<Position, Domain>(
            [dt, &registry](auto e, Position& pos, Domain& domain) {
                if (pos.pos.x < domain.ax || pos.pos.x > domain.bx ||
                    pos.pos.y < domain.ay || pos.pos.y > domain.by) {
                    registry.destroy(e);
                }
            });
    }

    /**
     * @brief Counter tracking the number of update calls.
     *
     * This counter increments each time onUpdate() is called.
     * Useful for debugging, profiling, or unit testing the system.
     *
     * @remarks This is a simple counter with no reset mechanism.
     *          Reset manually if needed for profiling purposes.
     */
    int updateCount = 0;
};
}  // namespace GameEngine