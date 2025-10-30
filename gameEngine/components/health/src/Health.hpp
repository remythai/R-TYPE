#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
/**
 * @struct Health
 * @brief Component representing an entity's health points and maximum capacity.
 *
 * Tracks both current and maximum health values for damage/healing systems.
 * Entities typically die or are destroyed when currentHp reaches zero.
 *
 * @details
 * **Health Management:**
 * - `currentHp`: Active health that decreases with damage
 * - `maxHp`: Upper limit for healing, defines entity robustness
 * - Invariant: `0 ≤ currentHp ≤ maxHp` (enforced by game systems)
 *
 * **Common Patterns:**
 * - Damage: `currentHp -= damage.dmg`
 * - Healing: `currentHp = min(currentHp + heal, maxHp)`
 * - Death check: `if (currentHp <= 0) destroyEntity()`
 * - Health bar: `fillRatio = currentHp / maxHp`
 *
 * **Use Cases:**
 * - Player health: Tracked and displayed in UI
 * - Enemy health: Determines hits needed to defeat
 * - Destructible objects: Walls, crates, obstacles
 * - Boss phases: Different behaviors at health thresholds
 *
 * @inherits Component<Health> for ECS registration.
 *
 * @example
 * ```cpp
 * // Player starts with full health
 * entity.add<Health>(100, 100);
 *
 * // Weak enemy with low health
 * entity.add<Health>(10, 10);
 *
 * // Boss with high health pool
 * entity.add<Health>(500, 500);
 *
 * // Damaged entity (e.g., loaded from save)
 * entity.add<Health>(45, 100);
 * ```
 *
 * @example
 * ```cpp
 * // Typical damage processing:
 * auto& health = entity.get<Health>();
 * health.currentHp -= 25;  // Take 25 damage
 * if (health.currentHp <= 0) {
 *     // Trigger death/destruction
 * }
 *
 * // Healing with cap:
 * health.currentHp = std::min(health.currentHp + 20, health.maxHp);
 * ```
 *
 * @note
 * - Constructor uses float but stores as int (implicit cast)
 * - Systems should clamp currentHp to [0, maxHp] range
 * - maxHp can be modified for power-ups or level scaling
 *
 * @see Damage
 * @see OnPickup
 */
struct Health : public Component<Health>
{
    /**
     * @brief Current health points remaining.
     *
     * Decreases when damaged, increases when healed.
     * Entity typically dies when this reaches zero.
     * Should never exceed maxHp.
     */
    int currentHp;

    /**
     * @brief Maximum health capacity.
     *
     * Upper limit for currentHp after healing.
     * Can be increased by power-ups or upgrades.
     * Represents entity's base robustness.
     */
    int maxHp;

    /**
     * @brief Constructs a Health component with specified values.
     *
     * @param val_currentHp Initial current health (default: 0).
     * @param val_maxHp Maximum health capacity (default: 0).
     *
     * @post Component initialized with specified health values.
     *
     * @note Parameters are float but stored as int (legacy API).
     * @warning Ensure val_currentHp ≤ val_maxHp for consistency.
     */
    Health(float val_currentHp = 0, float val_maxHp = 0)
        : currentHp(val_currentHp), maxHp(val_maxHp)
    {
    }

    /**
     * @brief Component name for reflection and debugging.
     */
    static constexpr const char* Name = "Health";

    /**
     * @brief Component version for serialization compatibility.
     */
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
