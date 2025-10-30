#pragma once
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

#include "../../../ecs/Component.hpp"
#include "../../../ecs/utils.hpp"

namespace GameEngine {
/**
 * @struct Renderable
 * @brief Component containing all data necessary for sprite rendering and
 * animation.
 *
 * Stores sprite sheet information, texture coordinates, animation frames,
 * and screen dimensions. Central component for the rendering system.
 *
 * @details
 * **Rendering Pipeline:**
 * 1. **Sprite Sheet**: Loads texture from spriteSheetPath
 * 2. **Frame Selection**: Uses currentRectPos to select texture region
 * 3. **Size Definition**: rectSize defines the sprite dimensions
 * 4. **Animation**: Animation system cycles through rectPos frames
 *
 * **Texture Coordinates:**
 * - `currentRectPos`: Top-left corner of current frame in sprite sheet
 * - `rectPos`: Vector of all animation frame positions
 * - `rectSize`: Width and height of each frame
 * - Coordinates in pixels within the sprite sheet texture
 *
 * **Animation Control:**
 * - `autoAnimate`: If true, Animation system processes this entity
 * - `frameDuration`: Milliseconds per animation frame
 * - `rectPos.size()`: Number of frames in animation loop
 *
 * **Screen Boundaries:**
 * - `screenSizeX/Y`: Used by Motion system for position clamping
 * - Prevents entities from moving off-screen
 * - Should match actual window/viewport dimensions
 *
 * **Sprite Sheet Format:**
 * ```
 * [Frame0][Frame1][Frame2]
 * [Frame3][Frame4][Frame5]
 * ```
 * Each frame position specified as vec2(x, y) in rectPos.
 *
 * @inherits Component<Renderable> for ECS registration.
 *
 * @performance
 * - Move semantics used for spriteSheetPath and rectPos
 * - No dynamic allocations during rendering
 * - Texture loaded once and cached by rendering system
 *
 * @example
 * ```cpp
 * // Static sprite (no animation)
 * entity.add<Renderable>(
 *     1920, 1080,                    // Screen size
 *     "assets/player.png",           // Sprite sheet path
 *     {{0, 0}},                      // Single frame at origin
 *     vec2(32, 32),                  // 32x32 pixel sprite
 *     0,                             // No frame duration needed
 *     false                          // No auto-animation
 * );
 * ```
 *
 * @example
 * ```cpp
 * // Animated walking cycle (4 frames, 100ms each)
 * entity.add<Renderable>(
 *     1920, 1080,                    // Screen size
 *     "assets/walk_cycle.png",       // Sprite sheet
 *     {                              // 4 animation frames
 *         vec2(0, 0),   vec2(32, 0),
 *         vec2(64, 0),  vec2(96, 0)
 *     },
 *     vec2(32, 48),                  // 32x48 sprite size
 *     100,                           // 100ms per frame = 10 FPS
 *     true                           // Enable auto-animation
 * );
 * ```
 *
 * @example
 * ```cpp
 * // Multi-row sprite sheet
 * std::vector<vec2> frames;
 * for (int row = 0; row < 2; ++row) {
 *     for (int col = 0; col < 4; ++col) {
 *         frames.push_back(vec2(col * 64, row * 64));
 *     }
 * }
 * entity.add<Renderable>(
 *     1920, 1080,
 *     "assets/character.png",
 *     frames,                        // 8 frames total
 *     vec2(64, 64),
 *     150,
 *     true
 * );
 * ```
 *
 * @note
 * - Empty rectPos creates non-renderable entity
 * - currentRectPos auto-set by Animation system if autoAnimate = true
 * - Manually set currentRectPos for state-based animations (idle/walk/jump)
 * - screenSizeX/Y should be consistent across all Renderable components
 *
 * @attention
 * - spriteSheetPath must be valid; missing files cause rendering errors
 * - All frames in rectPos should fit within sprite sheet bounds
 * - frameDuration = 0 with autoAnimate = true causes division by zero
 *
 * @see Animation
 * @see Position
 * @see Motion
 * @see vec2
 */
struct Renderable : public Component<Renderable>
{
    /**
     * @brief Screen width in pixels for boundary calculations.
     *
     * Used by Motion system to clamp entity positions.
     * Should match the actual rendering window width.
     */
    float screenSizeX;

    /**
     * @brief Screen height in pixels for boundary calculations.
     *
     * Used by Motion system to clamp entity positions.
     * Should match the actual rendering window height.
     */
    float screenSizeY;

    /**
     * @brief File path to the sprite sheet texture.
     *
     * Relative or absolute path to PNG/JPG image file.
     * Loaded by rendering system and cached for performance.
     *
     * @example "assets/sprites/player.png"
     * @example "../resources/enemies/boss.png"
     */
    std::string spriteSheetPath;

    /**
     * @brief Current texture rectangle position being rendered.
     *
     * Top-left corner (x, y) of the active frame within sprite sheet.
     * Set by Animation system or manually for state changes.
     * Initially defaults to (0, 0) or first frame in rectPos.
     */
    vec2 currentRectPos;

    /**
     * @brief Vector of all animation frame positions in sprite sheet.
     *
     * Each vec2 represents the top-left corner of a frame.
     * Animation system cycles through these positions based on time.
     * Empty vector = static sprite, single frame = no animation.
     */
    std::vector<vec2> rectPos;

    /**
     * @brief Dimensions of each sprite frame (width, height).
     *
     * All frames in an animation must have the same size.
     * Used by rendering system to extract correct texture region.
     */
    vec2 rectSize;

    /**
     * @brief Duration of each animation frame in milliseconds.
     *
     * Controls animation speed: lower = faster, higher = slower.
     * Example: 100ms = 10 FPS, 50ms = 20 FPS, 200ms = 5 FPS.
     * Ignored if autoAnimate = false.
     */
    int frameDuration;

    /**
     * @brief Flag enabling automatic frame animation.
     *
     * If true: Animation system automatically cycles through rectPos frames.
     * If false: currentRectPos must be manually controlled.
     * Use false for state-based animations (idle vs walking).
     */
    bool autoAnimate;

    /**
     * @brief Default constructor creating an empty, non-renderable component.
     *
     * Initializes all fields to neutral values.
     * Typically used for entities that will set rendering data later.
     *
     * @post All numeric fields = 0, string empty, vectors empty, autoAnimate =
     * false.
     */
    Renderable()
        : screenSizeX(0),
          screenSizeY(0),
          spriteSheetPath(""),
          currentRectPos{0, 0},
          rectPos(),
          rectSize{0, 0},
          frameDuration(0),
          autoAnimate(false)
    {
    }

    /**
     * @brief Parameterized constructor for fully-specified renderable entities.
     *
     * @param val_screenSizeX Screen width for boundary constraints.
     * @param val_screenSizeY Screen height for boundary constraints.
     * @param val_spriteSheetPath Path to sprite sheet texture file.
     * @param val_rectPos Vector of animation frame positions (moved).
     * @param val_rectSize Dimensions of each sprite frame.
     * @param val_frameDuration Milliseconds per animation frame.
     * @param val_autoAnimate Enable automatic animation cycling.
     *
     * @post Component fully initialized and ready for rendering.
     *
     * @note Uses move semantics for spriteSheetPath and rectPos for
     * performance.
     */
    Renderable(
        float val_screenSizeX, float val_screenSizeY,
        std::string val_spriteSheetPath, std::vector<vec2> val_rectPos,
        vec2 val_rectSize, int val_frameDuration, bool val_autoAnimate)
        : screenSizeX(val_screenSizeX),
          screenSizeY(val_screenSizeY),
          spriteSheetPath(std::move(val_spriteSheetPath)),
          rectPos(std::move(val_rectPos)),
          rectSize(val_rectSize),
          frameDuration(val_frameDuration),
          autoAnimate(val_autoAnimate)
    {
    }

    /**
     * @brief Component name for reflection and debugging.
     */
    static constexpr const char* Name = "Renderable";

    /**
     * @brief Component version for serialization compatibility.
     */
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
