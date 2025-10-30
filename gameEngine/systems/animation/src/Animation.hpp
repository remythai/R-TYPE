#pragma once

#include <chrono>
#include <ratio>

#include "../../../components/renderable/src/Renderable.hpp"
#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"

namespace GameEngine {
/**
 * @class Animation
 * @brief System that manages sprite animation by cycling through texture
 * rectangles.
 *
 * The Animation system handles frame-based sprite animations by:
 * 1. **Timing**: Tracks elapsed time since system initialization
 * 2. **Frame calculation**: Determines current animation frame based on time
 * 3. **Rect updates**: Updates the Renderable component's current texture
 * rectangle
 *
 * This enables smooth sprite animations without per-entity timers.
 *
 * @details
 * **Animation Pipeline (per frame):**
 * 1. **Time Calculation**: Computes milliseconds since system start
 * 2. **Frame Selection**: Uses modulo arithmetic to cycle through frames
 * 3. **Rect Update**: Sets currentRectPos to the appropriate frame
 *
 * **Frame Selection Formula:**
 * ```
 * frameIndex = (elapsedMs / frameDuration) % totalFrames
 * ```
 * This creates a looping animation that repeats indefinitely.
 *
 * **Timing Behavior:**
 * - All entities share the same time reference (startPoint)
 * - Synchronizes animations across multiple entities
 * - Frame duration determines animation speed (ms per frame)
 *
 * @requires
 * - Renderable: Must contain non-empty rectPos vector for animation frames
 * - frameDuration: Must be set to control animation speed
 *
 * @performance
 * - Time Complexity: O(n) where n is number of animated entities
 * - Space Complexity: O(1) per entity
 * - Single time query per frame for all entities
 *
 * @note
 * - Entities with empty rectPos vectors are skipped
 * - All animations start synchronized at system creation
 * - Frame duration is in milliseconds
 *
 * @example
 * ```cpp
 * // Entity with 4 animation frames, 100ms per frame
 * // rectPos = [rect0, rect1, rect2, rect3]
 * // frameDuration = 100
 * // At 250ms: frame = (250 / 100) % 4 = 2 → displays rect2
 * ```
 *
 * @see Renderable
 * @see Registry
 */
class Animation : public System<Animation>
{
   private:
    /**
     * @brief Time point marking system initialization.
     *
     * Used as reference point for calculating elapsed time.
     * Set once during construction and never reset.
     *
     * @details Uses steady_clock for monotonic time measurement,
     * immune to system clock adjustments.
     */
    std::chrono::time_point
        std::chrono::steady_clock,
        std::chrono::duration<long, std::ratio<1, 1000000000>>>
        startPoint = std::chrono::steady_clock::now();

   public:
    /**
     * @brief Constructs the Animation system and declares component
     * requirements.
     *
     * Registers that this system requires the Renderable component
     * to access animation frame data.
     *
     * @post System is configured for sprite animation processing.
     */
    Animation()
    {
        requireComponents<GameEngine::Renderable>();
    }

    /**
     * @brief Updates all entity animations based on elapsed time.
     *
     * Processes each entity with a Renderable component:
     *
     * **Time Calculation:**
     * - Computes time delta from system initialization
     * - Converts to milliseconds for frame indexing
     *
     * **Frame Selection:**
     * - Divides elapsed time by frame duration
     * - Uses modulo to wrap around frame count
     * - Updates currentRectPos with selected frame
     *
     * **Safety Check:**
     * - Skips entities with empty rectPos vectors
     * - Prevents division by zero and invalid access
     *
     * @param registry Reference to the ECS Registry (unused but required by
     * interface).
     * @param dt Delta time since last update in seconds (unused in current
     * implementation).
     *
     * @details
     * **Frame Timing:**
     * - frameDuration is in milliseconds
     * - Lower values = faster animation
     * - Example: 100ms = 10 FPS, 50ms = 20 FPS
     *
     * **Synchronization:**
     * - All entities use same time reference
     * - Animations start in sync at system creation
     * - To desync, use per-entity time offsets
     *
     * @attention
     * - Entities without rectPos frames are silently skipped
     * - Frame duration must be > 0 to avoid division by zero
     * - Animation loops infinitely; no stop mechanism
     *
     * @example
     * ```cpp
     * // Walking animation: 8 frames, 80ms per frame
     * // Total loop time: 8 * 80ms = 640ms
     * // At 1000ms: frame = (1000 / 80) % 8 = 4
     * ```
     *
     * @see Registry::each
     * @see Renderable
     */
    void onUpdate(Registry& registry, float dt)
    {
        updateCount++;
        auto currentTimePoint = std::chrono::steady_clock::now();
        auto deltaTime = currentTimePoint - startPoint;

        registry.each<Renderable>([dt, deltaTime](auto e, Renderable& render) {
            if (render.rectPos.size() != 0)
                render.currentRectPos =
                    render.rectPos
                        [std::chrono::duration_cast<std::chrono::milliseconds>(
                             deltaTime)
                             .count() /
                         render.frameDuration % render.rectPos.size()];
        });
    }

    /**
     * @brief Counter tracking system execution calls.
     *
     * Increments each frame when onUpdate() is called. Useful for:
     * - Performance profiling and frame counting
     * - Unit testing verification
     * - Debugging animation timing
     *
     * @remarks Value persists throughout system lifetime; reset manually if
     * needed for testing.
     */
    int updateCount = 0;
};
}  // namespace GameEngine
