#pragma once

#include <bitset>
#include <cstdint>
#include <vector>

#include "../../../components/collider/src/Collider.hpp"
#include "../../../components/damage/src/Damage.hpp"
#include "../../../components/domain/src/Domain.hpp"
#include "../../../components/health/src/Health.hpp"
#include "../../../components/inputControlled/src/InputControlled.hpp"
#include "../../../components/position/src/Position.hpp"
#include "../../../components/renderable/src/Renderable.hpp"
#include "../../../components/velocity/src/Velocity.hpp"
#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"

namespace GameEngine {
/**
 * @class FPInputHandler
 * @brief System that processes player input and manages input-controlled
 * entities.
 *
 * The InputHandler system translates input commands into entity behavior by:
 * 1. Reading input states from InputControlled components
 * 2. Updating entity velocity based on directional inputs
 * 3. Creating projectiles when shoot input is detected
 *
 * This system is responsible for converting raw input into game mechanics,
 * serving as the bridge between player input and entity physics/behavior.
 *
 * @details
 * **Input Mapping:**
 * - Input 0: Move down (positive Y velocity)
 * - Input 1: Move up (negative Y velocity)
 * - Input 2: Move left (negative X velocity)
 * - Input 3: Move right (positive X velocity)
 * - Input 4: Shoot (creates projectile entity)
 *
 * **Projectile Creation:**
 * When input 4 (shoot) is detected, the system:
 * 1. Creates a new entity
 * 2. Attaches required components (Renderable, Health, Damage, etc.)
 * 3. Positions it at the player's current position
 * 4. Sets up physics (Velocity, Velocity)
 * 5. Configures collision (Collider with bitset)
 * 6. Restricts movement within domain boundaries
 *
 * @note Multiple inputs can be processed in a single frame, allowing
 *       simultaneous movement and shooting.
 *
 * @requires
 * - InputControlled: Provides input commands
 * - Velocity: Modified by input
 * - Renderable: Required for entity visibility (used for requirement checking)
 * - Position: Used when creating projectiles
 *
 * @see InputControlled
 * @see Velocity
 * @see Position
 * @see Renderable
 * @see Collider
 * @see Domain
 */
class FPInputHandler : public System<FPInputHandler>
{
   public:
    /**
     * @brief Constructs the InputHandler and declares its component
     * requirements.
     *
     * Registers that this system requires:
     * - InputControlled: Source of input commands
     * - Velocity: Physical property to modify
     * - Renderable: Visible player entity indicator
     *
     * The system only activates when entities have all three components.
     *
     * @post System is ready to process input-controlled entities.
     */
    FPInputHandler()
    {
        requireComponents<GameEngine::InputControlled, GameEngine::Velocity>();
    }

    /**
     * @brief Processes input commands and updates entity behavior accordingly.
     *
     * For each entity with InputControlled and Velocity components:
     * 1. Resets velocity to zero
     * 2. Iterates through pending input commands
     * 3. Updates velocity or spawns projectiles based on input type
     * 4. Clears input queue after processing
     *
     * @param registry Reference to the ECS Registry for entity management.
     * @param dt Delta time since last update in seconds (unused but required by
     * interface).
     *
     * @details
     * **Input Processing:**
     * - Direction inputs (0-3) modify the Velocity component
     * - Shoot input (4) creates a new projectile entity with full setup
     *
     * **Projectile Properties:**
     * - Position: Copied from player entity
     * - Velocity: (10.0, 10.0) pixels per second
     * - Velocity: 10.0 (2D scalar)
     * - Health: 1 HP (destroyed on first collision)
     * - Damage: 1 damage per hit
     * - Renderable: 22.28x22.28 sprite from
     * "assets/sprites/playerProjectiles.png"
     * - Collider: Bitset "01000000" (collision layer configuration)
     * - Domain: Restricted to (0, 0) to (1905, 1080) screen boundaries
     *
     * **Velocity Values:**
     * - Directional movement: 5.0 units per second²
     * - Movement range: ±5.0 on X and Y axes
     *
     * @attention
     * - Velocity is reset each frame, so movement inputs must be
     *   provided continuously to maintain motion.
     * - Multiple conflicting direction inputs in the same frame will
     *   overwrite each other (last input wins).
     *
     * @note Projectile spawn position is retrieved fresh each frame,
     *       allowing for dynamic player movement while shooting.
     *
     * @example
     * ```cpp
     * // Player presses UP and SHOOT simultaneously
     * // Result: velocity.y = -5.0, projectile spawned at player pos
     * ```
     *
     * @see Registry::each
     * @see Registry::create
     * @see Registry::emplace
     * @see InputControlled
     * @see Velocity
     */
    void onUpdate(Registry& registry, float dt)
    {
        updateCount++;
        registry.each<InputControlled, Velocity>(
            [dt, &registry](
                auto e, InputControlled& inputs, Velocity& velocity) {
                float jumpForce = 300.0f;

                if (!inputs.firstInput && inputs.inputs.size() != 0) {
                    std::cout << "firstInput\n";
                    inputs.firstInput = true;
                }
                for (auto& it : inputs.inputs) {
                    if (it == 4) {
                        /// @brief Shoot: Create projectile with full component
                        /// setup
                        velocity.y = -jumpForce;
                    }
                }
            });
    }

    /**
     * @brief Counter tracking system execution calls.
     *
     * Increments each frame when onUpdate() is called. Useful for:
     * - Performance analysis
     * - Unit testing verification
     * - Debugging input processing
     *
     * @remarks Persists across frames; reset manually if needed for testing.
     */
    int updateCount = 0;
};
}  // namespace GameEngine
