#pragma once

#include <algorithm>

#include "../../../components/acceleration/src/Acceleration.hpp"
#include "../../../components/collider/src/Collider.hpp"
#include "../../../components/position/src/Position.hpp"
#include "../../../components/renderable/src/Renderable.hpp"
#include "../../../components/velocity/src/Velocity.hpp"
#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"

namespace GameEngine {
/**
 * @class Motion
 * @brief System that simulates entity movement using physics principles.
 *
 * The Motion system implements a simplified physics engine that handles:
 * 1. **Deceleration**: Velocity naturally dampens when acceleration is zero
 * 2. **Acceleration**: Applies acceleration forces to velocity each frame
 * 3. **Speed clamping**: Constrains velocity to maximum speed limits
 * 4. **Position updates**: Translates entities based on current velocity
 * 5. **Boundary clamping**: Keeps entities within screen boundaries
 *
 * This creates natural, responsive movement with friction-like behavior.
 *
 * @details
 * **Physics Pipeline (per frame):**
 * 1. **Deceleration Phase**: Reduces velocity by 75% when no external force is
 * applied
 *    - Positive velocities approach zero from above
 *    - Negative velocities approach zero from below
 *    - This simulates air resistance/friction
 *
 * 2. **Acceleration Phase**: Adds acceleration values to velocity
 *    - Clamped to [-speedMax, +speedMax] range
 *    - Prevents unlimited velocity buildup
 *
 * 3. **Position Update Phase**: Translates entity position by velocity
 *    - Position clamped to screen bounds: [0, screenSize]
 *    - Uses Renderable component for screen dimensions
 *
 * **Deceleration Formula:**
 * ```
 * vel_new = vel - (vel * 0.75)
 *         = vel * 0.25
 * ```
 * This exponential decay creates smooth stopping behavior.
 *
 * @requires
 * - Position: Current location to update
 * - Velocity: Current movement speed with speedMax limit
 * - Acceleration: Forces to apply
 * - Renderable: Screen size for boundary constraints
 *
 * @performance
 * - Time Complexity: O(n) where n is number of moving entities
 * - Space Complexity: O(1) per entity
 * - No allocations or iterations beyond entity processing
 *
 * @see Position
 * @see Velocity
 * @see Acceleration
 * @see Renderable
 */
class Motion : public System<Motion>
{
   public:
    /**
     * @brief Constructs the Motion system and declares component requirements.
     *
     * Registers that this system requires all physics-related components:
     * - Position: Entity location
     * - Velocity: Current movement speed
     * - Acceleration: Force to apply
     * - Renderable: Screen boundary information
     *
     * The system only processes entities with all four components.
     *
     * @post System is configured for physics simulation.
     */
    Motion()
    {
        requireComponents<
            GameEngine::Position, GameEngine::Velocity,
            GameEngine::Acceleration, GameEngine::Renderable,
            GameEngine::Collider>();
    }

    /**
     * @brief Updates all entity positions based on physics simulation.
     *
     * Processes each entity with motion components through a three-phase
     * physics pipeline:
     *
     * **Phase 1: Deceleration (Natural Friction)**
     * - Reduces velocity by 75% per frame (multiplies by 0.25)
     * - Applies independently to X and Y axes
     * - Preserves sign: positive velocities stay positive, negative stay
     * negative
     * - Creates smooth deceleration when no input is applied
     *
     * **Phase 2: Acceleration (Force Application)**
     * - Adds acceleration components to velocity
     * - Clamps result to [-speedMax, +speedMax] range
     * - Prevents unlimited acceleration buildup
     *
     * **Phase 3: Position Update (Movement)**
     * - Translates position by velocity amount
     * - Clamps position to screen bounds using Renderable dimensions
     * - Ensures entities never leave the visible play area
     *
     * @param registry Reference to the ECS Registry (unused but required by
     * interface).
     * @param dt Delta time since last update in seconds (unused in current
     * implementation).
     *
     * @details
     * **Deceleration Behavior:**
     * - Velocity of +100 becomes +25 (reduction of 75)
     * - Velocity of -100 becomes -25 (reduction of 75)
     * - Velocity of +1 becomes +0.25 → eventually +0 (clamped)
     *
     * **Speed Limiting:**
     * - Applied after acceleration step
     * - Uses Velocity::speedMax parameter
     * - Prevents physics simulation instability
     *
     * **Boundary Constraints:**
     * - Screen position: [0, screenSizeX] and [0, screenSizeY]
     * - Entities cannot move beyond screen edges
     * - Uses Renderable component for screen resolution
     *
     * @attention
     * - Deceleration applies EVERY frame, not just when no input
     * - Multiple inputs in same frame accumulate additively
     * - Position clamping prevents off-screen rendering
     *
     * @note
     * - dt parameter is unused; update rate should be consistent
     * - Physics is frame-based, not time-stepped
     *
     * @example
     * ```cpp
     * // Entity with vel=(10, 0), acc=(0, 0), speedMax=20
     * // Frame 1: vel becomes (2.5, 0)
     * // Frame 2: vel becomes (0.625, 0)
     * // Frame 3: vel becomes (0, 0) → stops
     * ```
     *
     * @example
     * ```cpp
     * // Player at (500, 500) with vel=(0, 0), acc=(5, 0), speedMax=10
     * // Frame: vel becomes (5, 0), pos becomes (505, 500)
     * // Next frame: vel becomes (1.25, 0), pos becomes (506.25, 500)
     * ```
     *
     * @see Registry::each
     * @see Position
     * @see Velocity
     * @see Acceleration
     */
    void onUpdate(Registry& registry, float dt)
    {
        updateCount++;

        registry.each<Position, Velocity, Acceleration, Renderable, Collider>(
            [dt](
                auto e, Position& pos, Velocity& vel, Acceleration& acc,
                Renderable& render, Collider& collider) {
                // Phase 1: Acceleration (apply forces and clamp to speed limit)
                vel.x = std::clamp(vel.x + (acc.x * dt), -vel.speedMax, vel.speedMax);
                vel.y = std::clamp(vel.y + (acc.y * dt), -vel.speedMax, vel.speedMax);

                // Phase 2: Position update (translate and constrain to screen
                // bounds)
                pos.pos.x = std::clamp(
                    pos.pos.x + (vel.x * dt), float(0),
                    render.screenSizeX - collider.size.x);
                pos.pos.y = std::clamp(
                    pos.pos.y + (vel.y * dt), float(0),
                    render.screenSizeY - collider.size.y);

                // Phase 3: Deceleration (600 pixels reduction)
                if (acc.decceleration) {
                    vel.x = vel.x > 0 ? std::max(vel.x - (600.0F * dt), 0.0F) : std::min(vel.x + (600.0F * dt), 0.0F);
                    vel.y = vel.y > 0 ? std::max(vel.y - (600.0F * dt), 0.0F) : std::min(vel.y + (600.0F * dt), 0.0F);
                }
            });
    }

    /**
     * @brief Counter tracking system execution calls.
     *
     * Increments each frame when onUpdate() is called. Useful for:
     * - Performance profiling and frame counting
     * - Unit testing verification
     * - Debugging system activation
     *
     * @remarks Value persists throughout system lifetime; reset manually if
     * needed for testing.
     */
    int updateCount = 0;
};
}  // namespace GameEngine
