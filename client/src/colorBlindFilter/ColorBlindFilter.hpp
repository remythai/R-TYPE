/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** ColorBlindFilter.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>

namespace CLIENT {

enum class ColorBlindMode
{
    NONE,
    PROTANOPIA,
    DEUTERANOPIA,
    TRITANOPIA
};

class ColorBlindFilter
{
   public:
    ColorBlindFilter();
    ~ColorBlindFilter() = default;

    void setMode(ColorBlindMode mode);
    ColorBlindMode getMode() const
    {
        return _currentMode;
    }

    const sf::RenderStates* getRenderStates() const;
    bool isActive() const
    {
        return _currentMode != ColorBlindMode::NONE && _shaderLoaded;
    }

    static std::string getModeName(ColorBlindMode mode);

   private:
    void updateShader();

    ColorBlindMode _currentMode;
    sf::Shader _shader;
    bool _shaderLoaded;
};

}  // namespace CLIENT
