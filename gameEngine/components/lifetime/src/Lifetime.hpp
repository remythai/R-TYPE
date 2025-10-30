#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {
/**
 * @struct Lifetime
 * @brief Component for entities with limited lifespan.
 *
 * Tracks remaining time before entity is automatically destroyed.
 * Decremented each frame by lifetime management systems.
 *
 * @details
 * **Lifetime Management:**
 * - `time`: Remaining seconds until destruction
 * - Decremented by delta time each frame
 * - Entity destroyed when time ≤ 0
 *
 * **Common Use Cases:**
 * - Projectiles: Bullets disappear after travel time
 * - Particles: VFX elements fade after duration
 * - Temporary power-ups: Buff effects expire
 * - Timed spawners: Create entities then self-destruct
 * - Enemy corpses: Disappear after display duration
 *
 * **Typical Processing:**
 * ```cpp
 * lifetime.time -= deltaTime;
 * if (lifetime.time <= 0) {
 *     registry.destroy(entity);
 * }
 * ```
 *
 * **Time Units:**
 * - Stored in seconds (floating-point)
 * - Example: 2.5f = 2.5 seconds lifespan
 * - Negative values trigger immediate destruction
 *
 * @inherits Component<Lifetime> for ECS registration.
 *
 * @example
 * ```cpp
 * // Bullet that disappears after 3 seconds
 * bullet.add<Lifetime>(3.0f);
 *
 * // Particle effect lasting 0.5 seconds
 * particle.add<Lifetime>(0.5f);
 *
 * // Power-up that expires after 10 seconds
 * powerup.add<Lifetime>(10.0f);
 *
 * // Immediate destruction (time already expired)
 * entity.add<Lifetime>(0.0f);
 * ```
 *
 * @example
 * ```cpp
 * // Typical lifetime system implementation:
 * void LifetimeSystem::onUpdate(Registry& registry, float dt) {
 *     std::vector<Entity> toDestroy;
 *     
 *     registry.each<Lifetime>([&](auto e, Lifetime& lifetime) {
 *         lifetime.time -= dt;
 *         if (lifetime.time <= 0) {
 *             toDestroy.push_back(e);
 *         }
 *     });
 *     
 *     for (auto e : toDestroy) {
 *         registry.destroy(e);
 *     }
 * }
 * ```
 *
 * @note
 * - Destruction should occur after all frame logic completes
 * - Consider visual fadeout before destruction for polish
 * - Can be extended with callbacks for cleanup logic
 *
 * @see Registry::destroy
 */
struct Lifetime : public Component<Lifetime>
{
    /**
     * @brief Remaining time in seconds before entity destruction.
     *
     * Decremented by delta time each frame.
     * Entity is destroyed when this reaches or drops below zero.
     */
    float time;

    /**
     * @brief Constructs a Lifetime component with specified duration.
     *
     * @param val_time Lifespan duration in seconds (default: 0).
     *
     * @post Component initialized with specified remaining time.
     *
     * @note Zero or negative values cause immediate destruction.
     */
    Lifetime(float val_time = 0) : time(val_time) {}

    /**
     * @brief Component name for reflection and debugging.
     */
    static constexpr const char* Name = "Lifetime";

    /**
     * @brief Component version for serialization compatibility.
     */
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
