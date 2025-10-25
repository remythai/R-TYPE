#pragma once

#include <cstdint>

namespace GameEngine {
/**
 * @struct GameClock
 * @brief Fixed timestep simulation clock for consistent update timing.
 *
 * The `GameClock` manages time progression in a game or simulation loop.
 * It maintains a fixed timestep (e.g., 1/60 seconds) for deterministic updates,
 * while tracking total elapsed time, frame count, and interpolation alpha for
 * rendering.
 *
 * This allows the game logic to run at a consistent rate independent of
 * the actual frame rendering rate.
 */
struct GameClock
{
    /// @brief Total accumulated simulation time (in seconds).
    float totalTime = 0.0f;

    /// @brief Fixed timestep duration (in seconds). Default = 1/60 = 0.01666...
    float fixedDeltaTime = 1.0f / 120.0f;

    /// @brief Number of fixed update frames that have occurred since start.
    uint64_t frameCount = 0;

    /// @brief Time scaling factor (1.0 = normal speed, 0.5 = half speed, 2.0 =
    /// double speed).
    float timeScale = 1.0f;

    /// @brief Accumulates real time between fixed updates.
    float accumulator = 0.0f;

    /**
     * @brief Advances the internal clock based on real-world delta time.
     *
     * The function increments the accumulator using real time (`realDt *
     * timeScale`) and determines how many fixed-timestep updates should occur
     * during this frame.
     *
     * It caps the number of fixed updates per frame to prevent a "spiral of
     * death" (where the simulation tries to catch up indefinitely after a large
     * frame hitch).
     *
     * @param realDt Real-world delta time (seconds since last frame).
     * @return The number of fixed simulation steps to perform this frame.
     */
    int update(float realDt)
    {
        accumulator += realDt * timeScale;

        int steps = 0;
        const int maxSteps = 5;

        while (accumulator >= fixedDeltaTime && steps < maxSteps) {
            totalTime += fixedDeltaTime;
            frameCount++;
            accumulator -= fixedDeltaTime;
            steps++;
        }

        return steps;
    }

    /**
     * @brief Returns the interpolation alpha between fixed updates.
     *
     * Used for smooth rendering between physics ticks.
     * Commonly used in render interpolation as:
     * `interpolated_position = lerp(previous, current,
     * getInterpolationAlpha())`
     *
     * @return A value between 0.0 and 1.0 representing interpolation progress.
     */
    float getInterpolationAlpha() const
    {
        return accumulator / fixedDeltaTime;
    }

    /**
     * @brief Returns the fixed timestep scaled by the time scale.
     *
     * @return Scaled fixed delta time (seconds).
     */
    float getFixedDeltaTime() const
    {
        return fixedDeltaTime * timeScale;
    }
};
}  // namespace GameEngine