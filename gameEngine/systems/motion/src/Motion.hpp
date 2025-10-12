#pragma once

#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"
#include <algorithm>

namespace GameEngine {
    /**
    * @class MotionSystem
    * @brief System responsible for updating entity motion (position, velocity, acceleration).
    *
    * The `MotionSystem` updates all entities that have the following components:
    * - `Position`
    * - `Velocity`
    * - `Acceleration`
    * - `Renderable`
    *
    * It applies acceleration and deceleration to velocity, then integrates
    * position accordingly while clamping movement within screen boundaries.
    *
    * ### Example Behavior:
    * - Applies **deceleration** based on velocity and maximum speed.
    * - Updates **velocity** according to acceleration and `dt`.
    * - Updates **position** based on velocity and time.
    * - Ensures position remains within the renderable area.
    */
    class MotionSystem : public System<MotionSystem> {
    public:
        /**
        * @brief Constructs the MotionSystem and declares its required components.
        *
        * Requires the following components for each processed entity:
        * - `GameEngine::Position`
        * - `GameEngine::Velocity`
        * - `GameEngine::Acceleration`
        * - `GameEngine::Renderable`
        */
        MotionSystem() {
            requireComponents<GameEngine::Position, GameEngine::Velocity, GameEngine::Acceleration, GameEngine::Renderable>();
        }
        
        /**
        * @brief Updates all entities matching the system’s component signature.
        *
        * For each entity:
        * 1. Applies **deceleration** to gradually reduce velocity when acceleration is zero.  
        * 2. Applies **acceleration** to modify velocity.  
        * 3. Updates the **position** based on velocity and time delta.  
        * 4. Clamps the entity’s position within render bounds (`Renderable::screenSizeX/Y`).
        *
        * @param registry The ECS registry providing access to components.
        * @param dt The elapsed time (in seconds) since the last update.
        */
        void onUpdate(Registry& registry, float dt) {
            updateCount++;

            registry.each<Position, Velocity, Acceleration, Renderable>(
                [dt](auto e, Position& pos, Velocity& vel, Acceleration& acc, Renderable& render) {

                    // --- Deceleration ---
                    // Gradually slow down entity movement over time.
                    vel.x = vel.x > 0
                        ? std::max(vel.x - (vel.x / 5) - (vel.speedMax / 5), float(0))
                        : std::min(vel.x + (vel.x / 5) + (vel.speedMax / 5), float(0));

                    vel.y = vel.y > 0
                        ? std::max(vel.y - (vel.y / 5) - (vel.speedMax / 5), float(0))
                        : std::min(vel.y + (vel.y / 5) + (vel.speedMax / 5), float(0));

                    // --- Acceleration ---
                    // Update velocity within allowed speed range.
                    vel.x = std::clamp(vel.x + acc.x * dt, -vel.speedMax, vel.speedMax);
                    vel.y = std::clamp(vel.y + acc.y * dt, -vel.speedMax, vel.speedMax);

                    // --- Position Integration ---
                    // Apply velocity to position, respecting screen bounds.
                    pos.x = std::clamp(pos.x + vel.x * dt, float(0), render.screenSizeX);
                    pos.y = std::clamp(pos.y + vel.y * dt, float(0), render.screenSizeY);
                }
            );
        }

        /// @brief Tracks how many times the system has updated (useful for debugging or profiling).
        int updateCount = 0;
    };
} // namespace GameEngine
