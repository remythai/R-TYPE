#pragma once

#include <bitset>
#include <cstdint>
#include <vector>
#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"
#include "../../../components/health/src/Health.hpp"
#include "../../../components/damage/src/Damage.hpp"
#include "../../../components/position/src/Position.hpp"
#include "../../../components/velocity/src/Velocity.hpp"
#include "../../../components/acceleration/src/Acceleration.hpp"
#include "../../../components/inputControlled/src/InputControlled.hpp"
#include "../../../components/renderable/src/Renderable.hpp"
#include "../../../components/domain/src/Domain.hpp"
#include "../../../components/collider/src/Collider.hpp"

namespace GameEngine {
    /**
     * @class InputHandler
     * @brief System that processes player input and manages input-controlled entities.
     *
     * The InputHandler system translates input commands into entity behavior by:
     * 1. Reading input states from InputControlled components
     * 2. Updating entity acceleration based on directional inputs
     * 3. Creating projectiles when shoot input is detected
     *
     * This system is responsible for converting raw input into game mechanics,
     * serving as the bridge between player input and entity physics/behavior.
     *
     * @details
     * **Input Mapping:**
     * - Input 0: Move down (positive Y acceleration)
     * - Input 1: Move up (negative Y acceleration)
     * - Input 2: Move left (negative X acceleration)
     * - Input 3: Move right (positive X acceleration)
     * - Input 4: Shoot (creates projectile entity)
     *
     * **Projectile Creation:**
     * When input 4 (shoot) is detected, the system:
     * 1. Creates a new entity
     * 2. Attaches required components (Renderable, Health, Damage, etc.)
     * 3. Positions it at the player's current position
     * 4. Sets up physics (Velocity, Acceleration)
     * 5. Configures collision (Collider with bitset)
     * 6. Restricts movement within domain boundaries
     *
     * @note Multiple inputs can be processed in a single frame, allowing
     *       simultaneous movement and shooting.
     *
     * @requires
     * - InputControlled: Provides input commands
     * - Acceleration: Modified by input
     * - Renderable: Required for entity visibility (used for requirement checking)
     * - Position: Used when creating projectiles
     *
     * @see InputControlled
     * @see Acceleration
     * @see Position
     * @see Renderable
     * @see Collider
     * @see Domain
     */
    class InputHandler : public System<InputHandler> {
    public:
        /**
         * @brief Constructs the InputHandler and declares its component requirements.
         *
         * Registers that this system requires:
         * - InputControlled: Source of input commands
         * - Acceleration: Physical property to modify
         * - Renderable: Visible player entity indicator
         *
         * The system only activates when entities have all three components.
         *
         * @post System is ready to process input-controlled entities.
         */
        InputHandler() {
            requireComponents<GameEngine::InputControlled, GameEngine::Acceleration, GameEngine::Renderable>();
        }
        
        /**
         * @brief Processes input commands and updates entity behavior accordingly.
         *
         * For each entity with InputControlled and Acceleration components:
         * 1. Resets acceleration to zero
         * 2. Iterates through pending input commands
         * 3. Updates acceleration or spawns projectiles based on input type
         * 4. Clears input queue after processing
         *
         * @param registry Reference to the ECS Registry for entity management.
         * @param dt Delta time since last update in seconds (unused but required by interface).
         *
         * @details
         * **Input Processing:**
         * - Direction inputs (0-3) modify the Acceleration component
         * - Shoot input (4) creates a new projectile entity with full setup
         *
         * **Projectile Properties:**
         * - Position: Copied from player entity
         * - Velocity: (10.0, 10.0) pixels per second
         * - Acceleration: 10.0 (2D scalar)
         * - Health: 1 HP (destroyed on first collision)
         * - Damage: 1 damage per hit
         * - Renderable: 22.28x22.28 sprite from "assets/sprites/playerProjectiles.png"
         * - Collider: Bitset "01000000" (collision layer configuration)
         * - Domain: Restricted to (0, 0) to (1905, 1080) screen boundaries
         *
         * **Acceleration Values:**
         * - Directional movement: 5.0 units per second²
         * - Movement range: ±5.0 on X and Y axes
         *
         * @attention
         * - Acceleration is reset each frame, so movement inputs must be
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
         * // Result: acceleration.y = -5.0, projectile spawned at player pos
         * ```
         *
         * @see Registry::each
         * @see Registry::create
         * @see Registry::emplace
         * @see InputControlled
         * @see Acceleration
         */
        void onUpdate(Registry& registry, float dt) {
            updateCount++;
            registry.each<InputControlled, Acceleration>([dt, &registry](auto e, InputControlled& inputs, Acceleration& acceleration) {
                float accelerationValue = 5.0;
                GameEngine::Position playerPos;
                uint32_t shoot = -1;
                acceleration.x = 0;
                acceleration.y = 0;
                std::vector<vec2> rectPos;
                
                for (auto &it : inputs.inputs) {
                    switch (it) {
                        case 0:
                            /// @brief Move down
                            acceleration.y = accelerationValue;
                            break;
                        case 1:
                            /// @brief Move up
                            acceleration.y = -accelerationValue;
                            break;
                        case 2:
                            /// @brief Move left
                            acceleration.x = -accelerationValue;
                            break;
                        case 3:
                            /// @brief Move right
                            acceleration.x = accelerationValue;
                            break;
                        case 4:
                            /// @brief Shoot: Create projectile with full component setup
                            shoot = registry.create();
                            rectPos.clear();
                            rectPos.push_back(vec2{0.0F, 0.0F});
                            rectPos.push_back(vec2{19.0F, 0.0F});
                            rectPos.push_back(vec2{38.0F, 0.0F});
                            registry.emplace<GameEngine::Renderable>(shoot, 1920.0, 1080.0, "assets/sprites/playerProjectiles.png", rectPos, vec2{22.28f, 22.28f}, 0.05f, true);
                            registry.emplace<GameEngine::Health>(shoot, 1, 1);
                            registry.emplace<GameEngine::Damage>(shoot, 1);
                            registry.emplace<GameEngine::Velocity>(shoot, 10.0, 10.0);
                            registry.emplace<GameEngine::Acceleration>(shoot, 10.0);
                            playerPos = registry.get<GameEngine::Position>(e);
                            registry.emplace<GameEngine::Position>(shoot, playerPos.pos.x, playerPos.pos.y);
                            registry.emplace<GameEngine::Collider>(shoot, vec2(0.0, 0.0), std::bitset<8>("01000000"), vec2(22.28, 22.28));
                            registry.emplace<GameEngine::Domain>(shoot, 0, 0, 1905.0, 1080.0);
                            break;
                        default:
                            break;
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
}
