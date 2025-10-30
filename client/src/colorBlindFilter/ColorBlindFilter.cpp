/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** ColorBlindFilter.cpp
*/

#include "ColorBlindFilter.hpp"

#include <iostream>

/**
 * @brief GLSL fragment shader source for simulating color blindness.
 *
 * This shader applies color transformation to simulate different types
 * of color blindness (protanopia, deuteranopia, tritanopia) based on
 * the selected mode.
 *
 * @details
 * The shader defines:
 * - A uniform sampler2D `texture` representing the source image.
 * - A uniform integer `mode` indicating the type of color blindness:
 *   - 0: No color blindness (original colors)
 *   - 1: Protanopia simulation
 *   - 2: Deuteranopia simulation
 *   - 3: Tritanopia simulation
 *
 * Transformation functions:
 * - `applyProtanopia(vec3 color)`: Applies a matrix transformation to simulate
 *   protanopia (red-green deficiency affecting red cones).
 * - `applyDeuteranopia(vec3 color)`: Applies a matrix transformation to simulate
 *   deuteranopia (red-green deficiency affecting green cones).
 * - `applyTritanopia(vec3 color)`: Applies a matrix transformation to simulate
 *   tritanopia (blue-yellow deficiency affecting blue cones).
 *
 * The `main()` function:
 * - Reads the color of the current fragment from `texture`.
 * - Applies the appropriate color transformation based on `mode`.
 * - Outputs the resulting color to `gl_FragColor`.
 *
 * @note This shader must be loaded as a fragment shader and requires
 *       system support for GLSL shaders.
 */

const std::string COLORBLIND_SHADER = R"(
uniform sampler2D texture;
uniform int mode;

vec3 applyProtanopia(vec3 color) {
    mat3 transform = mat3(
        0.567, 0.433, 0.0,
        0.558, 0.442, 0.0,
        0.0, 0.242, 0.758
    );
    return transform * color;
}

vec3 applyDeuteranopia(vec3 color) {
    mat3 transform = mat3(
        0.625, 0.375, 0.0,
        0.7, 0.3, 0.0,
        0.0, 0.3, 0.7
    );
    return transform * color;
}

vec3 applyTritanopia(vec3 color) {
    mat3 transform = mat3(
        0.95, 0.05, 0.0,
        0.0, 0.433, 0.567,
        0.0, 0.475, 0.525
    );
    return transform * color;
}

void main() {
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
    vec3 color = pixel.rgb;
    
    if (mode == 1) {
        color = applyProtanopia(color);
    } else if (mode == 2) {
        color = applyDeuteranopia(color);
    } else if (mode == 3) {
        color = applyTritanopia(color);
    }
    
    gl_FragColor = vec4(color, pixel.a);
}
)";

/**
 * @brief Default constructor for the ColorBlindFilter class.
 *
 * This constructor initializes a ColorBlindFilter object with a default
 * color blindness mode and attempts to load the color-blindness correction
 * shader from memory.
 *
 * The constructor performs the following actions:
 * - Initializes the current color blindness mode to @ref ColorBlindMode::NONE.
 * - Checks if shaders are supported on the current system using 
 *   `sf::Shader::isAvailable()`.
 * - If shaders are available, attempts to load the fragment shader from 
 *   memory using the shader source defined by @ref COLORBLIND_SHADER.
 * - If the shader loads successfully:
 *   - Sets the `_shaderLoaded` flag to `true`.
 *   - Initializes the shader uniform `"mode"` to `0` (corresponding to 
 *     `ColorBlindMode::NONE`).
 *   - Logs a success message to the standard output.
 * - If the shader fails to load or shaders are not supported:
 *   - Logs an appropriate error message to the standard error output.
 *
 * @note The shader used is a fragment shader intended to simulate or correct 
 * various types of color blindness depending on the mode selected.
 *
 * @warning If shaders are not supported or fail to load, the filter will not 
 * apply any color-blindness correction effects.
 *
 * @see ColorBlindMode
 * @see sf::Shader
 */

CLIENT::ColorBlindFilter::ColorBlindFilter()
    : _currentMode(ColorBlindMode::NONE), _shaderLoaded(false)
{
    if (sf::Shader::isAvailable()) {
        if (_shader.loadFromMemory(
                COLORBLIND_SHADER, sf::Shader::Type::Fragment)) {
            _shaderLoaded = true;
            _shader.setUniform("mode", 0);
            std::cout << "[ColorBlindFilter] Shader loaded successfully\n";
        } else {
            std::cerr << "[ColorBlindFilter] Failed to load shader\n";
        }
    } else {
        std::cerr
            << "[ColorBlindFilter] Shaders not supported on this system\n";
    }
}

/**
 * @brief Sets the current color blindness mode for the filter.
 *
 * This function updates the internal color blindness mode and refreshes
 * the shader to apply the new mode.
 *
 * @param mode The desired color blindness mode to apply.
 *             See @ref ColorBlindMode for available modes.
 *
 * @details
 * After setting the mode, the shader uniform `"mode"` is updated via
 * `updateShader()`, and a message is printed to standard output indicating
 * the new mode and its integer value.
 *
 * @see updateShader()
 * @see getModeName()
 */
void CLIENT::ColorBlindFilter::setMode(ColorBlindMode mode)
{
    _currentMode = mode;
    updateShader();
    std::cout << "[ColorBlindFilter] Mode changed to: " << getModeName(mode)
              << " (mode=" << static_cast<int>(mode) << ")\n";
}

/**
 * @brief Updates the shader uniform to match the current color blindness mode.
 *
 * @details
 * If the shader has been successfully loaded, this function sets the
 * shader's `"mode"` uniform to the integer value of `_currentMode`.
 * This ensures that the shader applies the correct color transformation
 * during rendering.
 *
 * @note Has no effect if the shader failed to load during construction.
 */
void CLIENT::ColorBlindFilter::updateShader()
{
    if (_shaderLoaded) {
        _shader.setUniform("mode", static_cast<int>(_currentMode));
    }
}

/**
 * @brief Returns the SFML render states configured with the color blindness shader.
 *
 * @return Pointer to an `sf::RenderStates` object containing the shader, or
 *         `nullptr` if the shader is not loaded or the current mode is NONE.
 *
 * @details
 * This function can be used when drawing SFML objects to automatically apply
 * the color blindness filter. If no shader should be applied (mode NONE or
 * shader not loaded), it returns `nullptr` so that default rendering occurs.
 *
 * @note The returned pointer is valid only as long as the `ColorBlindFilter`
 *       instance exists.
 *
 * @see sf::RenderStates
 */
const sf::RenderStates* CLIENT::ColorBlindFilter::getRenderStates() const
{
    if (!_shaderLoaded || _currentMode == ColorBlindMode::NONE) {
        return nullptr;
    }

    static sf::RenderStates states;
    states.shader = &_shader;
    return &states;
}

/**
 * @brief Returns a human-readable name for a given color blindness mode.
 *
 * @param mode The color blindness mode for which the name is requested.
 *
 * @return A `std::string` containing the name of the mode:
 * - "Aucun" for `ColorBlindMode::NONE`
 * - "Protanopia (Red-Green)" for `ColorBlindMode::PROTANOPIA`
 * - "Deuteranopia (Red-Green)" for `ColorBlindMode::DEUTERANOPIA`
 * - "Tritanopia (Blue-Yellow)" for `ColorBlindMode::TRITANOPIA`
 * - "Inconnu" for any undefined or invalid mode
 *
 * @details
 * This function is primarily used for logging and debugging purposes to
 * convert the enum value of the color blindness mode into a readable string.
 *
 * @note The returned string is localized (French for "Aucun" and "Inconnu") 
 *       for the NONE and unknown modes.
 *
 * @see ColorBlindMode
 */
std::string CLIENT::ColorBlindFilter::getModeName(ColorBlindMode mode)
{
    switch (mode) {
        case ColorBlindMode::NONE:
            return "Aucun";
        case ColorBlindMode::PROTANOPIA:
            return "Protanopia (Red-Green)";
        case ColorBlindMode::DEUTERANOPIA:
            return "Deuteranopia (Red-Green)";
        case ColorBlindMode::TRITANOPIA:
            return "Tritanopia (Blue-Yellow)";
        default:
            return "Inconnu";
    }
}

