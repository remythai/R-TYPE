/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** ColorBlindFilter.cpp
*/

#include "ColorBlindFilter.hpp"
#include <iostream>

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

CLIENT::ColorBlindFilter::ColorBlindFilter()
    : _currentMode(ColorBlindMode::NONE), _shaderLoaded(false)
{
    if (sf::Shader::isAvailable()) {
        if (_shader.loadFromMemory(COLORBLIND_SHADER, sf::Shader::Type::Fragment)) {
            _shaderLoaded = true;
            _shader.setUniform("mode", 0);
            std::cout << "[ColorBlindFilter] Shader loaded successfully\n";
        } else {
            std::cerr << "[ColorBlindFilter] Failed to load shader\n";
        }
    } else {
        std::cerr << "[ColorBlindFilter] Shaders not supported on this system\n";
    }
}

void CLIENT::ColorBlindFilter::setMode(ColorBlindMode mode)
{
    _currentMode = mode;
    updateShader();
    std::cout << "[ColorBlindFilter] Mode changed to: " << getModeName(mode) << " (mode=" << static_cast<int>(mode) << ")\n";
}

void CLIENT::ColorBlindFilter::updateShader()
{
    if (_shaderLoaded) {
        _shader.setUniform("mode", static_cast<int>(_currentMode));
    }
}

const sf::RenderStates* CLIENT::ColorBlindFilter::getRenderStates() const
{
    if (!_shaderLoaded || _currentMode == ColorBlindMode::NONE) {
        return nullptr;
    }
    
    static sf::RenderStates states;
    states.shader = &_shader;
    return &states;
}

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
