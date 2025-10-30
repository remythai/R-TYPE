#pragma once

#include "../../../ecs/Component.hpp"
#include "../../../ecs/utils.hpp"

namespace GameEngine {
/**
 * @struct Position
 * @brief Component representing an entity's 2D location in world space.
 *
 * Stores the spatial coordinates of an entity using a vec2 structure.
 * Fundamental component used by rendering, physics, and collision systems.
 *
 * @details
 * **Coordinate System:**
 * - Origin (0, 0): Typically top-left corner of screen
 * - Positive X: Rightward direction
 * - Positive Y: Downward direction (screen coordinates)
 * - Units: Pixels in screen space
 *
 * **Common Usage Patterns:**
 * - **Rendering**: Determines where sprite is drawn
 * - **Physics**: Updated by Motion system based on velocity
 * - **Collision**: Used to calculate collision box positions
 * - **AI**: Pathfinding and target tracking
 *
 * **Position Updates:**
 * ```cpp
 * // Typical physics update:
 * pos.pos.x += velocity.x;
 * pos.pos.y += velocity.y;
 * ```
 *
 * **Boundary Constraints:**
 * - May be clamped to screen bounds by Motion system
 * - May be constrained to Domain boundaries
 * - Usually kept within [0, screenSize] range
 *
 * @inherits Component<Position> for ECS registration.
 *
 * @example
 * ```cpp
 * // Player spawned at center of 800×600 screen
 * player.add<Position>(400, 300);
 *
 * // Enemy spawned at top-left
 * enemy.add<Position>(50, 50);
 *
 * // Projectile created at player's current position
 * auto& playerPos = player.get<Position>();
 * projectile.add<Position>(playerPos.pos.x, playerPos.pos.y);
 * ```
 *
 * @example
 * ```cpp
 * // Accessing and modifying position:
 * auto& pos = entity.get<Position>();
 * pos.pos.x += 10.0f;  // Move 10 pixels right
 * pos.pos.y -= 5.0f;   // Move 5 pixels up
 *
 * // Using vec2 operators:
 * vec2 offset(20, 30);
 * pos.pos += offset;  // Move by offset vector
 * ```
 *
 * @note
 * - Position is separate from visual rendering offset
 * - Collider may have originTranslation offset from Position
 * - Consider using double precision for large world coordinates
 *
 * @see vec2
 * @see Velocity
 * @see Motion
 * @see Renderable
 * @see Collider
 */
struct Position : public Component<Position>
{
    /**
     * @brief 2D position vector storing X and Y coordinates.
     *
     * Represents entity's location in world/screen space.
     * Modified by physics systems and used by rendering.
     */
    vec2 pos;

    /**
     * @brief Constructs a Position component at specified coordinates.
     *
     * @param val_x X coordinate in pixels (default: 0).
     * @param val_y Y coordinate in pixels (default: 0).
     *
     * @post Component initialized with vec2 at (val_x, val_y).
     */
    Position(float val_x = 0, float val_y = 0) : pos(val_x, val_y) {}

    /**
     * @brief Component name for reflection and debugging.
     */
    static constexpr const char* Name = "Position";

    /**
     * @brief Component version for serialization compatibility.
     */
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
