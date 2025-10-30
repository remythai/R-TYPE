/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** ColorBlindFilter.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>

namespace CLIENT {

/**
 * @enum ColorBlindMode
 * @brief Defines different modes of color blindness simulation.
 *
 * Used to determine which visual transformation is applied by the
 * ColorBlindFilter shader.
 *
 * @values
 * - NONE: No filter applied (normal vision)
 * - PROTANOPIA: Red-blindness simulation
 * - DEUTERANOPIA: Green-blindness simulation
 * - TRITANOPIA: Blue-blindness simulation
 *
 * @see ColorBlindFilter
 */
enum class ColorBlindMode
{
    NONE,          ///< Normal vision, no color correction.
    PROTANOPIA,    ///< Red color blindness simulation.
    DEUTERANOPIA,  ///< Green color blindness simulation.
    TRITANOPIA     ///< Blue color blindness simulation.
};

/**
 * @class ColorBlindFilter
 * @brief Applies a color-blindness simulation shader to the rendered scene.
 *
 * This class encapsulates an SFML shader that transforms the rendered colors
 * to simulate different types of color blindness (Protanopia, Deuteranopia,
 * Tritanopia).
 *
 * It can be used to make games more accessible by allowing users to preview
 * visuals under various color perception conditions.
 *
 * @details
 * **Behavior:**
 * - Loads a GLSL shader at runtime using `sf::Shader::loadFromMemory`.
 * - Sets a uniform `mode` indicating the color-blind mode.
 * - Provides a `RenderStates` reference that can be passed to
 *   `sf::RenderWindow::draw()` to apply the filter.
 *
 * **Shader Uniforms:**
 * - `texture`: Input texture to modify.
 * - `mode`: Integer corresponding to a ColorBlindMode.
 *
 * @example
 * ```cpp
 * CLIENT::ColorBlindFilter filter;
 * filter.setMode(CLIENT::ColorBlindMode::DEUTERANOPIA);
 * window.draw(sprite, *filter.getRenderStates());
 * ```
 *
 * @note Requires SFML shaders to be available on the system.
 *
 * @see sf::Shader
 * @see ColorBlindMode
 */
class ColorBlindFilter
{
   public:
    /**
     * @brief Constructs a new ColorBlindFilter and loads the shader.
     *
     * Initializes the shader from memory and sets the default mode to NONE.
     * If shaders are not supported, the filter will remain inactive.
     *
     * @post Shader is ready for use if available on the system.
     */
    ColorBlindFilter();
    ~ColorBlindFilter() = default;

    /**
     * @brief Changes the current color blindness simulation mode.
     *
     * Updates the shader’s internal uniform and modifies rendering accordingly.
     *
     * @param mode The new color blindness mode to apply.
     *
     * @see updateShader()
     */
    void setMode(ColorBlindMode mode);

    /**
     * @brief Returns the current color blindness mode.
     * @return The active mode (NONE if inactive).
     */
    ColorBlindMode getMode() const
    {
        return _currentMode;
    }

    /**
     * @brief Returns the render states associated with the shader.
     *
     * Can be passed to SFML draw calls to apply the filter.
     *
     * @return Pointer to `sf::RenderStates` containing the shader.
     */
    const sf::RenderStates* getRenderStates() const;

    /**
     * @brief Checks if the filter is active and available.
     * @return True if a mode is applied and shader loaded successfully.
     */
    bool isActive() const
    {
        return _currentMode != ColorBlindMode::NONE && _shaderLoaded;
    }

    /**
     * @brief Converts a ColorBlindMode enum value to a human-readable name.
     *
     * @param mode The mode to convert.
     * @return std::string containing the name of the mode.
     */
    static std::string getModeName(ColorBlindMode mode);

   private:
    /**
     * @brief Updates the shader’s uniform based on the current mode.
     *
     * Internal helper called automatically when setMode() is invoked.
     */
    void updateShader();

    ColorBlindMode _currentMode;  ///< Currently active color blindness mode.
    sf::Shader _shader;           ///< Underlying SFML shader object.
    bool _shaderLoaded;  ///< Indicates if the shader was successfully loaded.
};

}  // namespace CLIENT
