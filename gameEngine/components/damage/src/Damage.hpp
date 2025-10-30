#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
/**
 * @struct Damage
 * @brief Component representing the amount of damage an entity inflicts.
 *
 * Stores a damage value typically applied during collision events.
 * Used by combat systems to modify health/integrity of other entities.
 *
 * @details
 * **Common Use Cases:**
 * - Projectiles: Damage dealt on impact
 * - Enemies: Contact damage to player
 * - Hazards: Environmental damage zones
 * - Power-ups: Negative damage (healing)
 *
 * **Damage Processing:**
 * - Usually combined with Collider component
 * - Applied when collision is detected
 * - May be modified by armor/resistance systems
 *
 * @inherits Component<Damage> for ECS registration.
 *
 * @example
 * ```cpp
 * // Enemy bullet deals 10 damage
 * entity.add<Damage>(10);
 *
 * // Health pickup (negative damage heals)
 * entity.add<Damage>(-25);
 *
 * // Boss deals heavy damage
 * entity.add<Damage>(50);
 * ```
 *
 * @note
 * - Negative values can represent healing effects
 * - Zero damage entities might be used for detection triggers
 *
 * @see Health
 * @see Collider
 * @see Collision (system)
 */
struct Damage : public Component<Damage>
{
    /**
     * @brief Amount of damage dealt by this entity.
     *
     * Positive values reduce target health.
     * Negative values can represent healing or shields.
     */
    int dmg;

    /**
     * @brief Constructs a Damage component with specified damage value.
     *
     * @param val_dmg Damage amount (default: 0).
     *
     * @post Component initialized with specified damage value.
     */
    Damage(int val_dmg = 0) : dmg(val_dmg) {}

    /**
     * @brief Component name for reflection and debugging.
     */
    static constexpr const char* Name = "Damage";

    /**
     * @brief Component version for serialization compatibility.
     */
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
