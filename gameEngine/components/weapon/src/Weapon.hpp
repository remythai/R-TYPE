#pragma once

#include "Component.hpp"

namespace GameEngine {
struct Weapon : public Component<Weapon>
{
    enum class WeaponType
    {
        Pistol,
        Rifle,
        Laser,
        Rocket
    };

    WeaponType type;
    float fireRate;
    int projectileCount;

    Weapon(
        WeaponType weaponType = WeaponType::Pistol, float rate = 1.0f,
        int count = 1)
        : type(weaponType), fireRate(rate), projectileCount(count)
    {
    }

    static constexpr const char* Name = "Weapon";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
