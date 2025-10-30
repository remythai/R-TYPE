#pragma once

#include <bitset>
#include <cstdint>
#include <vector>

#include "../../../components/scoreValue/src/ScoreValue.hpp"
#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"

namespace GameEngine {

/**
 * @class ApplyScore
 * @brief System that applies score values when entities lose all their health.
 *
 * The ApplyScore system monitors entities with both **ScoreValue** and
 * **Health** components. Whenever an entity's health reaches zero,
 * its associated score value is added to the global game score.
 *
 * This system automates score accumulation based on gameplay events
 * such as enemy destruction, item collection, or objective completion.
 *
 * @details
 * **Behavior:**
 * - Iterates over all entities with ScoreValue and Health components.
 * - If `health.currentHp == 0`, adds `score.points` to `registry.score`.
 * - Does not remove entities or reset their score after processing
 *   (handled by other systems if needed).
 *
 * @note
 * The system is purely reactive and stateless, aside from a simple
 * call counter used for testing or debugging.
 *
 * @requires
 * - **ScoreValue**: Defines how many points an entity is worth.
 * - **Health**: Used to determine when an entity should contribute to the score.
 *
 * @see ScoreValue
 * @see Health
 * @see Registry
 */
class ApplyScore : public System<ApplyScore>
{
   public:
    /**
     * @brief Constructs the ApplyScore system and declares its component requirements.
     *
     * Registers that this system requires:
     * - ScoreValue: The amount of score awarded when the entity dies.
     * - Health: To check if the entity has been destroyed.
     *
     * @post System is ready to update the game score based on entity states.
     */
    ApplyScore()
    {
        requireComponents<GameEngine::ScoreValue, GameEngine::Health>();
    }

    /**
     * @brief Updates the global score based on destroyed entities.
     *
     * Iterates through all entities containing both **ScoreValue** and **Health**.
     * If an entity's `Health::currentHp` equals zero, its `ScoreValue::points`
     * are added to `registry.score`.
     *
     * @param registry Reference to the ECS Registry.
     * @param dt Delta time (unused).
     *
     * @details
     * **Example:**
     * ```cpp
     * // Example entity data:
     * // ScoreValue.points = 200
     * // Health.currentHp = 0
     * // Result: registry.score += 200
     * ```
     *
     * @note This system does not modify or remove entities; it only affects
     *       the global score counter.
     */
    void onUpdate(Registry& registry, float dt)
    {
        updateCount++;
        registry.each<ScoreValue, Health>(
            [dt, &registry](auto e, ScoreValue& score, Health& health) {
                if (health.currentHp == 0) {
                    registry.score += score.points;
                }
            });
    }

    /**
     * @brief Counter tracking the number of times onUpdate() has been called.
     *
     * Useful for:
     * - Verifying system execution frequency
     * - Unit testing
     * - Debugging ECS scheduling
     */
    int updateCount = 0;
};

}  // namespace GameEngine

