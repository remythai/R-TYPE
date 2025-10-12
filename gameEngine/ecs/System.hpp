#pragma once

#include "Types.hpp"
#include "ComponentRegistry.hpp"
#include <string>
#include <vector>

class Registry;

/**
 * @class ISystem
 * @brief Base interface for all systems within the ECS framework.
 *
 * Systems define logic that operates on entities with specific component combinations.
 * Each system updates once per frame (or tick) and processes entities that match its
 * required component signature.
 *
 * This abstract base defines the common interface that all derived systems must implement.
 */
class ISystem {
    public:
        virtual ~ISystem() = default;

        /**
        * @brief Executes the system’s logic.
        * 
        * Implementations should define how entities are processed using the `Registry`.
        * 
        * @param registry The ECS registry managing all entities and components.
        * @param dt Delta time in seconds since the last update.
        */
        virtual void update(Registry& registry, float dt) = 0;

        /**
        * @brief Returns the component signature required by this system.
        * 
        * The signature determines which entities are relevant to this system,
        * based on the components they possess.
        * 
        * @return The system’s required component signature.
        */
        virtual const ComponentSignature& getSignature() const = 0;

        /**
        * @brief Returns the system’s unique ID.
        * @return System identifier.
        */
        virtual SystemID getSystemID() const = 0;

        /**
        * @brief Returns the human-readable system name.
        * @return Name of the system.
        */
        virtual const std::string& getName() const = 0;

        /**
        * @brief Assigns a unique system ID (usually handled by the `Registry`).
        * @param id New system ID.
        */
        virtual void setSystemID(SystemID id) = 0;

        /// @brief Determines update order (higher priority systems run earlier).
        int priority = 0;

        /// @brief Enables or disables system execution.
        bool enabled = true;

        /// @brief Indicates whether this system’s required components are registered.
        bool hasRequiredComponents = false;
};

/**
 * @class System
 * @brief CRTP (Curiously Recurring Template Pattern) base class for ECS systems.
 *
 * This template class provides a convenient base for defining game systems.
 * Derived classes should implement:
 * ```cpp
 * void onUpdate(Registry& registry, float dt);
 * ```
 *
 * Example usage:
 * ```cpp
 * class MovementSystem : public System<MovementSystem> {
 * public:
 *     MovementSystem() {
 *         requireComponents<Transform, Velocity>();
 *     }
 *     void onUpdate(Registry& registry, float dt);
 * };
 * ```
 *
 * @tparam Derived The derived system type (used via CRTP).
 */
template<typename Derived>
class System : public ISystem {
    public:
        /**
        * @brief Calls the derived system’s `onUpdate` method if enabled and valid.
        * 
        * Ensures that system logic is executed only when the system is both active
        * (`enabled == true`) and all required components exist in the registry.
        *
        * @param registry ECS registry.
        * @param dt Delta time since the last frame.
        */
        void update(Registry& registry, float dt) override {
            if (enabled && hasRequiredComponents) {
                static_cast<Derived*>(this)->onUpdate(registry, dt);
            }
        }

        /**
        * @brief Returns the system’s required component signature.
        */
        const ComponentSignature& getSignature() const override {
            return signature;
        }

        /**
        * @brief Returns the system’s unique identifier.
        */
        SystemID getSystemID() const override {
            return systemID;
        }

        /**
        * @brief Returns the system’s human-readable name.
        */
        const std::string& getName() const override {
            return name;
        }

        /**
        * @brief Assigns a unique system ID (usually set by the registry).
        * @param id New system ID.
        */
        void setSystemID(SystemID id) override {
            systemID = id;
        }

        /**
        * @brief Assigns a human-readable name to the system.
        * @param n The name to assign.
        */
        void setName(const std::string& n) {
            name = n;
        }

    protected:
        /**
        * @brief Declares which components this system requires.
        *
        * Entities must have all listed components to be processed by the system.
        * This version uses compile-time component type inference.
        *
        * Example:
        * ```cpp
        * requireComponents<Transform, Velocity>();
        * ```
        */
        template<typename... Components>
        void requireComponents() {
            signature.reset();
            (signature.set(ComponentRegistry::instance().getOrCreateID<Components>()), ...);
        }

        /**
        * @brief Declares required components using their names as strings.
        *
        * Useful for runtime configuration (e.g., editor tools or scripting systems).
        *
        * @param componentNames List of component names to require.
        */
        void requireComponentsByName(const std::vector<std::string>& componentNames) {
            signature.reset();
            for (const auto& name : componentNames) {
                ComponentID id = ComponentRegistry::instance().getOrCreateID(name);
                if (id != ComponentRegistry::INVALID_ID) {
                    signature.set(id);
                }
            }
        }

    private:
        /// @brief Bitset defining which components an entity must have to match this system.
        ComponentSignature signature;

        /// @brief System’s unique identifier (assigned by the registry).
        SystemID systemID = 0;

        /// @brief Human-readable name for debugging, profiling, or editor display.
        std::string name;

        /// @brief Grant `Registry` access to internal data.
        friend class Registry;
};