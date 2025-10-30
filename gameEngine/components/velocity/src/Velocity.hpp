#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
/**
 * @struct Velocity
 * @brief Component representing an entity's movement speed with maximum
 * constraints.
 *
 * Stores current velocity in pixels per frame along X and Y axes,
 * plus a maximum speed limit to prevent infinite acceleration.
 *
 * @details
 * **Velocity Mechanics:**
 * - `x, y`: Current movement speed (pixels per frame)
 * - `speedMax`: Upper limit for velocity magnitude
 * - Modified by Acceleration component each frame
 * - Used by Motion system to update Position
 *
 * **Physics Pipeline:**
 * 1. Deceleration reduces velocity (friction)
 * 2. Acceleration adds force to velocity
 * 3. Velocity clamped to [-speedMax, +speedMax]
 * 4. Position updated by velocity amount
 *
 * **Direction Convention:**
 * - Positive X: Rightward movement
 * - Negative X: Leftward movement
 * - Positive Y: Downward movement (screen coords)
 * - Negative Y: Upward movement
 *
 * **Speed Limiting:**
 * ```cpp
 * // Ensures velocity never exceeds bounds:
 * vel.x = clamp(vel.x, -vel.speedMax, vel.speedMax);
 * vel.y = clamp(vel.y, -vel.speedMax, vel.speedMax);
 * ```
 *
 * **Typical Speed Values:**
 * - Slow (tanks, heavy units): 1-3 pixels/frame
 * - Normal (players, enemies): 5-10 pixels/frame
 * - Fast (projectiles, dashing): 15-30 pixels/frame
 * - Very fast (lasers, missiles): 50+ pixels/frame
 *
 * @inherits Component<Velocity> for ECS registration.
 *
 * @performance
 * - Clamping occurs every frame in Motion system
 * - Speed limit prevents physics instability
 * - Deceleration creates smooth stopping behavior
 *
 * @example
 * ```cpp
 * // Player with moderate speed limit
 * player.add<Velocity>(8.0f, 0.0f, 0.0f);  // Max 8 px/frame, currently still
 *
 * // Fast-moving bullet
 * bullet.add<Velocity>(20.0f, 15.0f, 0.0f);  // Max 20 px/frame, moving right
 *
 * // Slow enemy patrol
 * enemy.add<Velocity>(3.0f, -2.0f, 0.0f);  // Max 3 px/frame, moving left
 * ```
 *
 * @example
 * ```cpp
 * // Diagonal movement (combining X and Y):
 * auto& vel = entity.get<Velocity>();
 * vel.x = 5.0f;   // Move right
 * vel.y = -5.0f;  // Move up
 * // Resulting movement: diagonal up-right at ~7.07 px/frame
 * ```
 *
 * @example
 * ```cpp
 * // Dynamic speed boost power-up:
 * void applySpeedBoost(Entity entity, float multiplier) {
 *     auto& vel = entity.get<Velocity>();
 *     vel.speedMax *= multiplier;  // Temporary speed increase
 *     // Remember to restore original speedMax after duration
 * }
 * ```
 *
 * @note
 * - speedMax is per-axis limit, not total magnitude
 * - Diagonal movement can exceed speedMax (√2 × speedMax)
 * - Zero velocity with speedMax > 0 allows future acceleration
 * - Constructor order: speedMax first, then current velocities
 *
 * @attention
 * - speedMax = 0 prevents all movement
 * - Very high speedMax may cause tunneling (passing through walls)
 * - Negative speedMax creates undefined behavior
 *
 * @see Acceleration
 * @see Position
 * @see Motion
 */
struct Velocity : public Component<Velocity>
{
    /**
     * @brief Current horizontal velocity in pixels per frame.
     *
     * Positive values move entity rightward, negative leftward.
     * Clamped to [-speedMax, +speedMax] by Motion system.
     */
    float x;

    /**
     * @brief Current vertical velocity in pixels per frame.
     *
     * Positive values move entity downward, negative upward.
     * Clamped to [-speedMax, +speedMax] by Motion system.
     */
    float y;

    /**
     * @brief Maximum allowed velocity magnitude per axis.
     *
     * Upper bound for velocity components after acceleration.
     * Prevents infinite speed buildup and ensures stable physics.
     * Applied independently to X and Y axes.
     */
    float speedMax;

    /**
     * @brief Constructs a Velocity component with specified parameters.
     *
     * @param val_speedMax Maximum velocity limit (default: 10.0).
     * @param val_x Initial horizontal velocity (default: 0).
     * @param val_y Initial vertical velocity (default: 0).
     *
     * @post Component initialized with speed limit and current velocity.
     *
     * @note Parameter order differs from member declaration for logical
     * grouping.
     */
    Velocity(float val_speedMax = 10.0f, float val_x = 0, float val_y = 0)
        : x(val_x), y(val_y), speedMax(val_speedMax)
    {
    }

    /**
     * @brief Component name for reflection and debugging.
     */
    static constexpr const char* Name = "Velocity";

    /**
     * @brief Component version for serialization compatibility.
     */
    static constexpr const char* Version = "1.0.0";
};

}  // namespace GameEngine
