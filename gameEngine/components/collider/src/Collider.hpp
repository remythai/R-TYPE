#pragma once

#include <bitset>
#include <variant>

#include "../../../ecs/Component.hpp"
#include "../../../ecs/utils.hpp"

namespace GameEngine {
/**
 * @struct HitBox
 * @brief Marker type for collision detection (currently unused).
 *
 * Reserved for future collision system extensions.
 */
struct HitBox
{};

/**
 * @struct Collider
 * @brief Component defining collision boundaries and interaction rules.
 *
 * Specifies a rectangular collision area with:
 * - Position offset from entity origin
 * - Size dimensions (width × height)
 * - Collision layer masks for selective detection
 *
 * @details
 * **Collision Filtering:**
 * - `entitySelector`: Layers this collider belongs to (bitset)
 * - `entityDiff`: Layers this collider can collide with (bitset)
 * - Collision occurs when: `(A.selector & B.diff) != 0`
 *
 * **Spatial Definition:**
 * - `originTranslation`: Offset from entity's Position component
 * - `size`: Width (X) and height (Y) of collision box
 * - Actual collision bounds: `[pos + translation, pos + translation + size]`
 *
 * **Layer System (8-bit):**
 * - Bit 0: Player entities
 * - Bit 1: Enemy entities
 * - Bit 2: Projectiles
 * - Bit 3-7: Custom game-specific layers
 *
 * @inherits Component<Collider> for ECS registration.
 *
 * @example
 * ```cpp
 * // Player collider: belongs to layer 0, collides with layers 1 and 2
 * Collider playerCol(
 *     vec2(0, 0),           // No offset
 *     std::bitset<8>("00000001"),  // Layer 0 (player)
 *     std::bitset<8>("00000110"),  // Collides with layers 1, 2
 *     vec2(32, 48)          // 32×48 pixel box
 * );
 *
 * // Enemy projectile: belongs to layer 2, collides with layer 0
 * Collider bulletCol(
 *     vec2(-2, -2),         // Centered offset
 *     std::bitset<8>("00000100"),  // Layer 2 (projectile)
 *     std::bitset<8>("00000001"),  // Collides with layer 0 (player)
 *     vec2(8, 8)            // 8×8 pixel box
 * );
 * ```
 *
 * @see Position
 * @see Collision (system)
 */
struct Collider : public Component<Collider>
{
    /**
     * @brief Offset from entity's Position to collision box origin.
     *
     * Allows collision box to be positioned differently from entity center.
     * Useful for fine-tuning hit detection (e.g., excluding decorative parts).
     */
    vec2 originTranslation;

    /**
     * @brief Bitset defining which collision layers this entity belongs to.
     *
     * Each bit represents a layer (0-7). An entity can belong to multiple
     * layers simultaneously. Used in conjunction with other entities'
     * entityDiff.
     */
    std::bitset<8> entitySelector;

    /**
     * @brief Bitset defining which collision layers this entity can collide
     * with.
     *
     * Collision occurs when: `(thisEntity.selector & otherEntity.diff) != 0`
     * Enables one-way collisions and layer-based filtering.
     */
    std::bitset<8> entityDiff;

    /**
     * @brief Dimensions of the collision box (width, height).
     *
     * Defines the rectangular collision area size in pixels.
     * Combined with originTranslation to form the full collision bounds.
     */
    vec2 size;

    /**
     * @brief Constructs a Collider component with specified parameters.
     *
     * @param val_translation Offset from entity position (default: origin).
     * @param val_selector Collision layers this entity belongs to (default:
     * none).
     * @param val_diff Collision layers this entity interacts with (default:
     * none).
     * @param val_size Collision box dimensions (default: zero size).
     *
     * @post Component initialized with specified collision properties.
     *
     * @note
     * - Zero-size colliders never trigger collisions
     * - Empty selector/diff bitsets disable all collision detection
     */
    Collider(
        vec2 val_translation = vec2(0, 0),
        std::bitset<8> val_selector = std::bitset<8>(0),
        std::bitset<8> val_diff = std::bitset<8>(0), vec2 val_size = vec2(0, 0))
        : originTranslation(val_translation),
          entitySelector(val_selector),
          entityDiff(val_diff),
          size(val_size)
    {
    }

    /**
     * @brief Component name for reflection and debugging.
     */
    static constexpr const char* Name = "Collider";

    /**
     * @brief Component version for serialization compatibility.
     */
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine