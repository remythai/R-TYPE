#pragma once

#include <cmath>
#include "../../../components/acceleration/src/Acceleration.hpp"
#include "../../../components/AIControlled/src/AIControlled.hpp"
#include "../../../components/position/src/Position.hpp"
#include "../../../components/velocity/src/Velocity.hpp"
#include "../../../components/renderable/src/Renderable.hpp"
#include "../../../components/collider/src/Collider.hpp"
#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"

namespace GameEngine {

/**
 * @struct SinusoidalPattern
 * @brief Component that stores sinusoidal movement parameters for an entity.
 *
 * This component marks an entity for sinusoidal movement and stores its
 * wave pattern configuration. Entities without this component will not
 * be affected by the SinusoidalAI system.
 */
struct SinusoidalPattern : public Component<SinusoidalPattern>
{
    /**
     * @brief Constructs a sinusoidal pattern configuration.
     *
     * @param amp Maximum vertical displacement from center path (in pixels)
     * @param freq Wave frequency in radians per pixel (higher = tighter waves)
     * @param phase Initial phase offset for wave variation between entities
     */
    SinusoidalPattern(float amp = 100.0f, float freq = 0.005f, float phase = 0.0f)
        : amplitude(amp), frequency(freq), phaseOffset(phase)
    {
    }

    float amplitude;     ///< Maximum vertical displacement
    float frequency;     ///< Wave tightness (radians per pixel)
    float phaseOffset;   ///< Phase shift for wave variation

    static constexpr const char* Name = "SinusoidalPattern";
    static constexpr const char* Version = "1.0.0";
};

/**
 * @class SinusoidalAI
 * @brief System that applies sinusoidal vertical movement to AI-controlled entities.
 *
 * This system creates wave-like movement patterns for enemies while maintaining
 * their horizontal velocity. The sinusoidal pattern is based on the entity's
 * X position, creating smooth, predictable wave motion.
 *
 * @details
 * **Movement Behavior:**
 * - Horizontal movement: Unchanged (managed by Motion system)
 * - Vertical movement: sin(x * frequency + phase) * amplitude
 * - Boundary-aware: Clamps amplitude to prevent screen overflow
 *
 * **Physics Integration:**
 * - Modifies Velocity.y directly for immediate effect
 * - Preserves existing horizontal velocity and acceleration
 * - Works in tandem with Motion system
 * - Bypasses Motion's deceleration by continuously updating velocity
 *
 * @requires
 * - AIControlled: Marker to identify AI entities
 * - SinusoidalPattern: Configuration for wave parameters
 * - Position: Current location for sine calculation
 * - Velocity: To apply vertical sinusoidal movement
 * - Renderable: Screen bounds for clamping
 * - Collider: Entity size for boundary calculations
 */
class SinusoidalAI : public System<SinusoidalAI>
{
   public:
    /**
     * @brief Constructs the SinusoidalAI system.
     *
     * The system processes only entities with both AIControlled and
     * SinusoidalPattern components, allowing selective application.
     */
    SinusoidalAI()
    {
        requireComponents<
            GameEngine::AIControlled, GameEngine::SinusoidalPattern,
            GameEngine::Position, GameEngine::Velocity,
            GameEngine::Renderable, GameEngine::Collider>();
    }

    /**
     * @brief Updates AI entities with sinusoidal vertical movement.
     *
     * Applies wave motion to each AI-controlled entity with SinusoidalPattern:
     * 1. Calculates sine wave based on X position and phase offset
     * 2. Derives velocity from position derivative (chain rule)
     * 3. Clamps to prevent screen boundary violations
     * 4. Overrides any vertical velocity from other sources
     *
     * @param registry ECS Registry containing all entities
     * @param dt Delta time since last update (unused - position-based wave)
     *
     * @details
     * **Wave Formula:**
     * ```
     * y_offset = amplitude * sin(x * frequency + phase)
     * velocity_y = amplitude * frequency * cos(x * frequency + phase) * velocity_x
     * ```
     *
     * **Derivative Explanation:**
     * Since position changes with velocity_x, we use the chain rule:
     * ```
     * dy/dt = dy/dx * dx/dt
     *       = [A*f*cos(fx+p)] * velocity_x
     * ```
     *
     * **Boundary Protection:**
     * - Calculates safe amplitude based on current Y position
     * - Reduces wave intensity near screen edges
     * - Prevents entities from clipping outside visible area
     * - Uses collider size for accurate boundary detection
     *
     * @note Wave is position-based, not time-based, so horizontal speed
     *       affects wave traversal rate naturally. Faster enemies = faster waves.
     *
     * @attention This system should run BEFORE Motion system to ensure
     *            the calculated velocity is applied before Motion's deceleration.
     */
    void onUpdate(Registry& registry, float dt)
    {
        registry.each<
            AIControlled, SinusoidalPattern, Position, Velocity, 
            Renderable, Collider>(
            [](auto e, AIControlled& ai, SinusoidalPattern& pattern,
               Position& pos, Velocity& vel, Renderable& render, 
               Collider& collider) {
                
                // Calculate safe amplitude to prevent screen overflow
                float topMargin = pos.pos.y;
                float bottomMargin = render.screenSizeY - pos.pos.y - collider.size.y;
                float safeAmplitude = std::min({
                    pattern.amplitude, 
                    topMargin - 10.0f,      // Keep 10px safety margin
                    bottomMargin - 10.0f
                });

                // Only apply sinusoidal if we have safe room to move
                if (safeAmplitude > 0.0f) {
                    // Calculate wave phase including offset for variation
                    float wavePhase = pos.pos.x * pattern.frequency + pattern.phaseOffset;
                    
                    // Calculate vertical velocity using chain rule derivative
                    // dy/dt = amplitude * frequency * cos(phase) * dx/dt
                    float verticalVelocity = safeAmplitude * pattern.frequency 
                                           * std::cos(wavePhase) 
                                           * std::abs(vel.x);

                    // Apply vertical velocity (overrides any previous Y velocity)
                    vel.y = verticalVelocity;
                } else {
                    // Too close to boundary, stop vertical movement
                    vel.y = 0.0f;
                }
            });
    }
};
}  // namespace GameEngine