#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
/**
 * @struct Domain
 * @brief Component defining a rectangular boundary constraint for entity
 * movement.
 *
 * Specifies a 2D rectangular area within which an entity is constrained.
 * Defines boundaries using two corner points (A and B) in screen coordinates.
 *
 * @details
 * **Boundary Definition:**
 * - Point A (ax, ay): Typically the top-left corner
 * - Point B (bx, by): Typically the bottom-right corner
 * - Valid area: [ax, bx] × [ay, by]
 *
 * **Common Use Cases:**
 * - Enemy patrol zones: Restrict movement to specific areas
 * - Player boundaries: Different from screen bounds
 * - Boss arenas: Confine boss to specific regions
 * - Safe zones: Areas where entities cannot enter/exit
 *
 * **Coordinate System:**
 * - Uses screen-space coordinates (pixels)
 * - Origin (0, 0) typically at top-left of screen
 * - Positive X: rightward, Positive Y: downward
 *
 * @inherits Component<Domain> for ECS registration.
 *
 * @note
 * - Ensure ax < bx and ay < by for proper rectangle definition
 * - May be used by movement systems to clamp positions
 * - Can overlap with screen boundaries or be subset of them
 *
 * @example
 * ```cpp
 * // Enemy confined to left half of 800×600 screen
 * entity.add<Domain>(0, 0, 400, 600);
 *
 * // Boss arena in center of screen
 * entity.add<Domain>(200, 150, 600, 450);
 *
 * // Vertical patrol corridor
 * entity.add<Domain>(350, 0, 450, 600);
 * ```
 *
 * @see Position
 * @see Motion
 */
struct Domain : public Component<Domain>
{
    /**
     * @brief X coordinate of first corner (typically left edge).
     */
    float ax;

    /**
     * @brief Y coordinate of first corner (typically top edge).
     */
    float ay;

    /**
     * @brief X coordinate of second corner (typically right edge).
     */
    float bx;

    /**
     * @brief Y coordinate of second corner (typically bottom edge).
     */
    float by;

    /**
     * @brief Constructs a Domain component with specified boundary corners.
     *
     * @param val_ax X coordinate of first corner (default: 0).
     * @param val_ay Y coordinate of first corner (default: 0).
     * @param val_bx X coordinate of second corner (default: 0).
     * @param val_by Y coordinate of second corner (default: 0).
     *
     * @post Component initialized with specified rectangular bounds.
     *
     * @warning Ensure val_ax < val_bx and val_ay < val_by for valid rectangle.
     */
    Domain(
        float val_ax = 0, float val_ay = 0, float val_bx = 0, float val_by = 0)
        : ax(val_ax), ay(val_ay), bx(val_bx), by(val_by)
    {
    }

    /**
     * @brief Component name for reflection and debugging.
     */
    static constexpr const char* Name = "Domain";

    /**
     * @brief Component version for serialization compatibility.
     */
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine