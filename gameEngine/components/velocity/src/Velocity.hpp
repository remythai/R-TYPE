#pragma once

#include "../../../ecs/Component.hpp"

namespace GameEngine {

/**
 * @struct Velocity
 * @brief Composant représentant la vélocité d'une entité.
 * 
 * Hérite de Component<Velocity> pour bénéficier automatiquement
 * de l'interface IComponent nécessaire au chargement dynamique.
 */
struct Velocity : public Component<Velocity> {
    float x;          ///< Vitesse horizontale
    float y;          ///< Vitesse verticale
    float speedMax;   ///< Vitesse maximale autorisée
    
    /**
     * @brief Constructeur avec initialisation.
     * @param val_speedMax Vitesse maximale
     * @param val_x Vitesse initiale X
     * @param val_y Vitesse initiale Y
     */
    Velocity(float val_speedMax = 10.0f, float val_x = 0, float val_y = 0)
        : x(val_x), y(val_y), speedMax(val_speedMax) {}
    
    // Métadonnées pour l'identification dynamique
    static constexpr const char* Name = "Velocity";
    static constexpr const char* Version = "1.0.0";
};

} // namespace GameEngine
