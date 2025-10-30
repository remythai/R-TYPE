#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
/**
 * @struct Gravity
 * @brief Component representing gravitational force applied to an entity.
 *
 * Stores a constant downward force value that is continuously applied
 * to the entity's vertical acceleration or velocity.
 *
 * @details
 * **Physics Behavior:**
 * - Typically applied as constant downward acceleration
 * - Simulates Earth-like gravity or custom gravity zones
 * - Usually added to Acceleration.y each frame
 *
 * **Force Magnitude:**
 * - Positive values: Pull downward (standard gravity)
 * - Negative values: Push upward (reverse gravity zones)
 * - Zero: No gravitational effect (space/zero-g areas)
 *
 * **Typical Values:**
 * - Realistic: 9.8 units/frame² (if 1 unit = 1 meter)
 * - Platformer: 0.3 - 1.0 (tuned for gameplay feel)
 * - Space shooter: 0.0 (no gravity)
 *
 * **Integration with Physics:**
 * ```cpp
 * // Typical usage in physics system:
 * acceleration.y += gravity.force;  // Per frame
 * ```
 *
 * @inherits Component<Gravity> for ECS registration.
 *
 * @example
 * ```cpp
 * // Standard downward gravity for platformer
 * entity.add<Gravity>(0.5f);
 *
 * // Heavy gravity for fast-falling objects
 * entity.add<Gravity>(1.2f);
 *
 * // Reverse gravity zone (ceiling walker)
 * entity.add<Gravity>(-0.5f);
 *
 * // Zero gravity (floating in space)
 * entity.add<Gravity>(0.0f);
 * ```
 *
 * @note
 * - Applied independently from other acceleration sources
 * - Can be combined with jump mechanics for platformers
 * - May be toggled on/off by removing/adding component
 *
 * @see Acceleration
 * @see Velocity
 * @see Motion
 */
struct Gravity : public Component<Gravity>
{
    /**
     * @brief Gravitational force magnitude in pixels per frame.
     *
     * Positive values pull downward, negative push upward.
     * Applied continuously each physics update.
     */
    float force;

    /**
     * @brief Constructs a Gravity component with specified force.
     *
     * @param val_force Gravitational force magnitude (default: 0).
     *
     * @post Component initialized with specified gravity force.
     */
    Gravity(float val_force = 0) : force(val_force) {}

    /**
     * @brief Component name for reflection and debugging.
     */
    static constexpr const char* Name = "Gravity";

    /**
     * @brief Component version for serialization compatibility.
     */
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
