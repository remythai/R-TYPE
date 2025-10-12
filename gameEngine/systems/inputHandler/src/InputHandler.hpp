#pragma once

#include <cstdint>
#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"
#include "../../../components/health/src/Health.hpp"
#include "../../../components/damage/src/Damage.hpp"
#include "../../../components/position/src/Position.hpp"
#include "../../../components/velocity/src/Velocity.hpp"
#include "../../../components/acceleration/src/Acceleration.hpp"
#include "../../../components/inputControlled/src/InputControlled.hpp"

namespace GameEngine {

    /**
    * @class InputHandlerSystem
    * @brief System responsible for translating player input into entity actions and motion.
    *
    * The `InputHandlerSystem` processes all entities that have:
    * - `InputControlled` — tracks current input commands.
    * - `Acceleration` — modified to apply directional movement.
    *
    * Depending on the inputs, the system:
    * - Updates the entity’s acceleration to move it up, down, left, or right.
    * - Handles shooting by spawning a projectile entity with the appropriate components
    *   (`Health`, `Damage`, `Velocity`, `Acceleration`, and `Position`).
    *
    * This system represents the bridge between player input and in-game motion/behavior.
    */
    class InputHandlerSystem : public System<InputHandlerSystem> {
    public:
        /**
        * @brief Constructs the input handler system and declares its component dependencies.
        *
        * Entities must have:
        * - `GameEngine::InputControlled`
        * - `GameEngine::Acceleration`
        */
        InputHandlerSystem() {
            requireComponents<GameEngine::InputControlled, GameEngine::Acceleration>();
        }

        /**
        * @brief Processes input for all entities and updates their acceleration or triggers actions.
        *
        * For each `InputControlled` entity:
        * - Resets acceleration to zero.
        * - Applies directional acceleration based on pressed keys.
        * - If the "shoot" input is detected, spawns a new projectile entity.
        *
        * The projectile is given:
        * - `Health(1, 1)`
        * - `Damage(1)`
        * - `Velocity(1000, 1000)`
        * - `Acceleration(1000)`
        * - `Position` matching the player's position.
        *
        * @param registry The ECS registry managing entities and components.
        * @param dt Delta time since the last update (unused here, but available for consistency).
        */
        void onUpdate(Registry& registry, float dt) {
            updateCount++;

            registry.each<InputControlled, Acceleration>(
                [dt, &registry](auto e, InputControlled& inputs, Acceleration& acceleration) {

                    const float accelerationValue = 1'000'000.0f; // Strength of acceleration
                    acceleration.x = 0;
                    acceleration.y = 0;

                    GameEngine::Position playerPos{};
                    uint32_t shoot = static_cast<uint32_t>(-1); // Invalid entity placeholder

                    // Process all input commands for the entity
                    for (auto& it : inputs.inputs) {
                        switch (it) {
                            case 0: // Move up
                                acceleration.y = accelerationValue;
                                break;
                            case 1: // Move down
                                acceleration.y = -accelerationValue;
                                break;
                            case 2: // Move left
                                acceleration.x = -accelerationValue;
                                break;
                            case 3: // Move right
                                acceleration.x = accelerationValue;
                                break;
                            case 4: // Shoot / Fire action
                                shoot = registry.create();
                                // Initialize projectile components
                                registry.emplace<GameEngine::Health>(shoot, 1, 1);
                                registry.emplace<GameEngine::Damage>(shoot, 1);
                                registry.emplace<GameEngine::Velocity>(shoot, 1000.0f, 1000.0f);
                                registry.emplace<GameEngine::Acceleration>(shoot, 1000.0f);
                                playerPos = registry.get<GameEngine::Position>(e);
                                registry.emplace<GameEngine::Position>(shoot, playerPos.x, playerPos.y);
                                break;
                            default:
                                break;
                        }
                    }
                }
            );
        }

        /// @brief Counts how many times this system has updated (useful for diagnostics or profiling).
        int updateCount = 0;
    };
} // namespace GameEngine
