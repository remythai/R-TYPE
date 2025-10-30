#pragma once

#include <string>
#include <vector>

#include "../../../ecs/Component.hpp"

namespace GameEngine {
/**
 * @struct InputControlled
 * @brief Component marking entities as controllable by player input.
 *
 * Tracks input events and state for entities controlled by keyboard, mouse,
 * or gamepad. Input systems query this component to determine which entities
 * respond to player commands.
 *
 * @details
 * **Input Management:**
 * - `inputs`: Queue/history of input codes received
 * - `firstInput`: Flag indicating if entity has received any input
 * - Processed by input systems each frame
 *
 * **Input Codes (typical mapping):**
 * - 0-9: Numeric keys
 * - 10-35: Letter keys (A-Z)
 * - 100+: Special keys (arrows, space, etc.)
 * - Negative: Mouse/gamepad inputs
 *
 * **Common Patterns:**
 * - Input buffer: Store multiple inputs for combo systems
 * - Input polling: Check current frame inputs
 * - Input history: Replay or undo mechanics
 * - First input detection: Tutorial triggers
 *
 * **Usage Workflow:**
 * 1. Input system detects key press/release
 * 2. Adds input code to `inputs` vector
 * 3. Sets `firstInput = true` on first input ever
 * 4. Game logic system reads and processes inputs
 * 5. Inputs vector cleared each frame or after processing
 *
 * @inherits Component<InputControlled> for ECS registration.
 *
 * @example
 * ```cpp
 * // Mark player entity as input-controlled
 * auto player = registry.create();
 * player.add<InputControlled>();
 *
 * // Later in input system:
 * auto& input = player.get<InputControlled>();
 * input.inputs.push_back(KEY_W);  // W key pressed
 * if (!input.firstInput) {
 *     input.firstInput = true;
 *     // Trigger "game started" event
 * }
 * ```
 *
 * @example
 * ```cpp
 * // Processing inputs in game logic:
 * registry.each<InputControlled, Acceleration>(
 *     [](auto e, InputControlled& input, Acceleration& acc) {
 *         for (int code : input.inputs) {
 *             if (code == KEY_LEFT) acc.x = -10.0f;
 *             if (code == KEY_RIGHT) acc.x = 10.0f;
 *             if (code == KEY_SPACE) /* fire weapon */
;
*
}
*input.inputs.clear();  // Process once per frame
*
});
 * ```
 *
 * @note
 * - Only one entity should typically have this component (player)
 * - Can support multiple players with different input mappings
 * - Consider input buffering for frame-perfect inputs
 *
 * @see Acceleration
 * @see Velocity
 */
struct InputControlled : public Component<InputControlled>
 {
     /**
      * @brief Vector storing input codes received this frame.
      *
      * Populated by input system, consumed by game logic.
      * Typically cleared each frame after processing.
      * Can store multiple inputs for combo detection.
      */
     std::vector<int> inputs;

     /**
      * @brief Flag indicating if entity has received input at least once.
      *
      * Used for:
      * - Tutorial triggers ("player has started playing")
      * - Lazy initialization of gameplay systems
      * - Distinguishing between "no input yet" vs "no input this frame"
      *
      * Set to true on first input and never reset.
      */
     bool firstInput;

     /**
      * @brief Constructs an InputControlled component with empty state.
      *
      * @post Component initialized with no inputs and firstInput = false.
      */
     InputControlled() : firstInput(false) {}

     /**
      * @brief Component name for reflection and debugging.
      */
     static constexpr const char* Name = "InputControlled";

     /**
      * @brief Component version for serialization compatibility.
      */
     static constexpr const char* Version = "1.0.0";
 };
 }  // namespace GameEngine
