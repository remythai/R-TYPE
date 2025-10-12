#pragma once

namespace GameEngine {
struct Weapon {
    enum class WeaponType { Pistol, Rifle, Laser, Rocket };
    WeaponType type{WeaponType::Pistol};
    float fireRate{1.0f};
    int projectileCount{1};
};
}
