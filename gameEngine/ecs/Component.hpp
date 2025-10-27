#pragma once

#include <memory>
#include <string>

#include "Types.hpp"

/**
 * @class IComponent
 * @brief Interface de base pour tous les composants ECS.
 *
 * Permet l'identification et la gestion dynamique des composants
 * chargés depuis des bibliothèques partagées.
 */
class IComponent
{
   public:
    virtual ~IComponent() = default;

    /**
     * @brief Retourne le nom unique du composant.
     * @return Nom du type de composant (ex: "Velocity", "Position")
     */
    virtual const char* getComponentName() const = 0;

    /**
     * @brief Retourne la version du composant.
     * @return Version (format: "1.0.0")
     */
    virtual const char* getVersion() const = 0;

    /**
     * @brief Clone le composant (copie profonde).
     * @return Pointeur vers une nouvelle instance clonée
     */
    virtual std::unique_ptr<IComponent> clone() const = 0;
};

/**
 * @class Component
 * @brief Classe de base CRTP pour les composants concrets.
 *
 * Simplifie l'implémentation des composants en fournissant
 * automatiquement les méthodes de l'interface.
 *
 * @tparam Derived Le type du composant dérivé
 *
 * @example
 * ```cpp
 * struct Velocity : public Component<Velocity> {
 *     float x, y;
 *     float speedMax;
 *
 *     Velocity(float max = 10.0f, float vx = 0, float vy = 0)
 *         : x(vx), y(vy), speedMax(max) {}
 *
 *     static constexpr const char* Name = "Velocity";
 *     static constexpr const char* Version = "1.0.0";
 * };
 * ```
 */
template <typename Derived>
class Component : public IComponent
{
   public:
    virtual ~Component() = default;

    const char* getComponentName() const override
    {
        return Derived::Name;
    }

    const char* getVersion() const override
    {
        return Derived::Version;
    }

    std::unique_ptr<IComponent> clone() const override
    {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};

/**
 * @brief Macro pour simplifier l'export de composants depuis une shared
 * library.
 *
 * Chaque bibliothèque de composant doit exposer une fonction de création.
 */
#define COMPONENT_EXPORT_API extern "C"

/**
 * @brief Type de fonction pour créer une instance de composant.
 */
using ComponentCreateFunc = IComponent* (*)();

/**
 * @brief Type de fonction pour détruire une instance de composant.
 */
using ComponentDestroyFunc = void (*)(IComponent*);