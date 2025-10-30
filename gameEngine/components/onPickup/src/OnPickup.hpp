#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
/**
 * @struct OnPickup
 * @brief Component defining power-up effects applied when entity is collected.
 *
 * Stores stat modifications and buffs that are transferred to the collecting
 * entity. Used for pickups, collectibles, and temporary power-ups in games.
 *
 * @details
 * **Pickup Effects:**
 * - **Instant bonuses**: Applied immediately on pickup
 *   - `hpBonus`: Heal amount (can be negative for damage pickups)
 *   - `hpMaxBonus`: Permanent max health increase
 * - **Temporary buffs**: Applied for limited duration
 *   - `dmgBonus`: Increase attack damage
 *   - `cooldownBonus`: Reduce ability cooldowns (negative = faster)
 *   - `scoreMultiplierBonus`: Increase points earned
 *   - `duration`: How long temporary buffs last (seconds)
 *
 * **Processing Workflow:**
 * 1. Collision detected between player and pickup entity
 * 2. OnPickup effects extracted from pickup entity
 * 3. Instant bonuses applied immediately to player stats
 * 4. Temporary buffs tracked with expiration timers
 * 5. Pickup entity destroyed after collection
 *
 * **Bonus Stacking:**
 * - Instant bonuses: Add/subtract from current values
 * - Temporary buffs: Multiple pickups may stack or override
 * - Duration: Each pickup may have independent timer
 *
 * @inherits Component<OnPickup> for ECS registration.
 *
 * @example
 * ```cpp
 * // Health pack: Restore 50 HP instantly
 * healthPack.add<OnPickup>(50, 0, 0, 0.0f, 0.0f, 0.0f);
 *
 * // Max health upgrade: Permanently increase max HP by 25
 * upgrade.add<OnPickup>(0, 25, 0, 0.0f, 0.0f, 0.0f);
 *
 * // Damage boost: +10 damage for 15 seconds
 * damagePickup.add<OnPickup>(0, 0, 10, 0.0f, 0.0f, 15.0f);
 *
 * // Score multiplier: 2x points for 20 seconds
 * multiplier.add<OnPickup>(0, 0, 0, 0.0f, 1.0f, 20.0f);
 *
 * // Rapid fire: -0.5s cooldown for 10 seconds
 * rapidFire.add<OnPickup>(0, 0, 0, -0.5f, 0.0f, 10.0f);
 *
 * // Super combo: Multiple effects for 30 seconds
 * superPickup.add<OnPickup>(100, 0, 20, -0.3f, 0.5f, 30.0f);
 * ```
 *
 * @example
 * ```cpp
 * // Typical pickup processing system:
 * void onCollision(Entity player, Entity pickup) {
 *     auto& onPickup = pickup.get<OnPickup>();
 *     auto& health = player.get<Health>();
 *
 *     // Apply instant bonuses
 *     health.currentHp += onPickup.hpBonus;
 *     health.maxHp += onPickup.hpMaxBonus;
 *
 *     // Register temporary buffs
 *     if (onPickup.duration > 0) {
 *         registerBuff(player, onPickup);
 *     }
 *
 *     // Remove pickup entity
 *     registry.destroy(pickup);
 * }
 * ```
 *
 * @note
 * - Duration of 0 means instant-only effects (no temporary buffs)
 * - Negative bonuses can create hazardous pickups (curses)
 * - Consider clamping health bonuses to prevent overflow
 * - scoreMultiplierBonus is additive (1.0 = +100% = 2x multiplier)
 *
 * @see Health
 * @see Damage
 * @see Collider
 */
struct OnPickup : public Component<OnPickup>
{
    /**
     * @brief Instant health restoration/damage amount.
     *
     * Added to currentHp immediately on pickup.
     * Positive values heal, negative values damage.
     */
    int hpBonus;

    /**
     * @brief Permanent maximum health increase.
     *
     * Added to maxHp immediately on pickup.
     * Allows currentHp to be healed above previous limit.
     */
    int hpMaxBonus;

    /**
     * @brief Temporary damage increase for attacks.
     *
     * Applied for `duration` seconds.
     * Positive values increase attack power.
     */
    int dmgBonus;

    /**
     * @brief Temporary ability cooldown reduction.
     *
     * Applied for `duration` seconds.
     * Negative values reduce cooldowns (faster abilities).
     * Example: -0.5f reduces 2s cooldown to 1.5s.
     */
    float cooldownBonus;

    /**
     * @brief Temporary score multiplier increase.
     *
     * Applied for `duration` seconds.
     * Additive: 0.5f adds +50% to base multiplier.
     * Example: Base 1.0x + 0.5f bonus = 1.5x score multiplier.
     */
    float scoreMultiplierBonus;

    /**
     * @brief Duration of temporary buffs in seconds.
     *
     * If > 0: dmgBonus, cooldownBonus, and scoreMultiplierBonus
     *         are active for this duration.
     * If = 0: Only instant bonuses (hpBonus, hpMaxBonus) apply.
     */
    float duration;

    /**
     * @brief Constructs an OnPickup component with specified effects.
     *
     * @param val_hpBonus Instant health change (default: 0).
     * @param val_hpMaxBonus Permanent max health increase (default: 0).
     * @param val_dmgBonus Temporary damage bonus (default: 0).
     * @param val_cooldownBonus Temporary cooldown reduction (default: 0).
     * @param val_scoreMultiplierBonus Temporary score multiplier (default: 0).
     * @param val_duration Buff duration in seconds (default: 0).
     *
     * @post Component initialized with specified pickup effects.
     */
    OnPickup(
        int val_hpBonus = 0, int val_hpMaxBonus = 0, int val_dmgBonus = 0,
        float val_cooldownBonus = 0.0f, float val_scoreMultiplierBonus = 0.0f,
        float val_duration = 0.0f)
        : hpBonus(val_hpBonus),
          hpMaxBonus(val_hpMaxBonus),
          dmgBonus(val_dmgBonus),
          cooldownBonus(val_cooldownBonus),
          scoreMultiplierBonus(val_scoreMultiplierBonus),
          duration(val_duration)
    {
    }

    /**
     * @brief Component name for reflection and debugging.
     */
    static constexpr const char* Name = "OnPickup";

    /**
     * @brief Component version for serialization compatibility.
     */
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
