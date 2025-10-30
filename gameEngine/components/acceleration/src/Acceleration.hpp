#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
/**
 * @struct Acceleration
 * @brief Component representing force applied to an entity's velocity.
 *
 * Stores acceleration values in pixels per frame along X and Y axes.
 * Used by physics systems (e.g., Motion) to modify entity velocity.
 *
 * @details
 * **Physics Behavior:**
 * - Positive X: Accelerate rightward
 * - Negative X: Accelerate leftward
 * - Positive Y: Accelerate downward
 * - Negative Y: Accelerate upward
 *
 * **Usage Patterns:**
 * - Player input: Set acceleration based on key presses
 * - Gravity: Constant downward acceleration (e.g., y = 9.8)
 * - Projectiles: Initial burst acceleration, then zero
 *
 * @inherits Component<Acceleration> for ECS registration.
 *
 * @example
 * ```cpp
 * // Player accelerates right when key pressed
 * entity.add<Acceleration>(10.0f, 0.0f);
 *
 * // Gravity affecting falling object
 * entity.add<Acceleration>(0.0f, 0.5f);
 * ```
 *
 * @see Velocity
 * @see Motion
 */
struct Acceleration : public Component<Acceleration>
{
    /**
     * @brief Horizontal acceleration in pixels per frame.
     *
     * Positive values accelerate rightward, negative leftward.
     */
    float x;

    /**
     * @brief Vertical acceleration in pixels per frame.
     *
     * Positive values accelerate downward (screen coordinates),
     * negative upward.
     */
    float y;

    /**
     * @brief Constructs an Acceleration component with specified forces.
     *
     * @param val_x Horizontal acceleration (default: 0).
     * @param val_y Vertical acceleration (default: 0).
     *
     * @post Component initialized with specified acceleration values.
     */
    Acceleration(float val_x = 0, float val_y = 0) : x(val_x), y(val_y) {}

    /**
     * @brief Component name for reflection and debugging.
     */
    static constexpr const char* Name = "Acceleration";

    /**
     * @brief Component version for serialization compatibility.
     */
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
