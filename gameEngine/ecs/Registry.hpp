#pragma once

#include "Types.hpp"
#include "ComponentRegistry.hpp"
#include "System.hpp"
#include "EntityManager.hpp"
#include "SparseSet.hpp"
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include "Clock.hpp"

/**
 * @class Registry
 * @brief Central class for managing entities, components, and systems.
 *
 * The Registry acts as the core of the ECS (Entity-Component-System) architecture.
 * It handles entity creation/destruction, component storage, and system execution.
 *
 * Components are stored in pools (SparseSets), while systems are dynamically added and updated.
 */
class Registry {
    public:
        using Entity = EntityManager::Entity;

        /**
        * @brief Constructs the registry, preparing internal storage.
        */
        Registry() {
            componentPools.reserve(MAX_COMPONENTS);
            systems.reserve(32);
            updateSystemAvailability();
        }

        /**
        * @brief Creates a new entity.
        * @return The newly created entity ID.
        */
        Entity create() {
            return entityManager.create();
        }

        /**
        * @brief Destroys an entity and removes all its components.
        * @param e Entity to destroy.
        */
        void destroy(Entity e) {
            for (auto& pool : componentPools) {
                if (pool) {
                    pool->remove(e);
                }
            }
            entityManager.destroy(e);
        }

        /**
        * @brief Adds a new component of type `Component` to entity `e`.
        * @tparam Component Component type to add.
        * @tparam Args Argument types for component constructor.
        * @param e Target entity.
        * @param args Arguments forwarded to component constructor.
        * @return Reference to the newly emplaced component.
        */
        template<typename Component, typename... Args>
        Component& emplace(Entity e, Args&&... args) {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();
            auto& pool = assurePool<Component>(id);

            bool wasEmpty = pool.size() == 0;

            pool.emplace(e, std::forward<Args>(args)...);

            // If it's the first component of that type, systems may become active
            if (wasEmpty) {
                std::cout << "[Registry] First component of type added, updating system availability\n";
                availableComponents.set(id);
                updateSystemAvailability();
            }
            
            return pool.get(e);
        }

        /**
        * @brief Removes a component of type `Component` from entity `e`.
        * @tparam Component Component type to remove.
        * @param e Target entity.
        */
        template<typename Component>
        void remove(Entity e) {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();
            
            if (id < componentPools.size() && componentPools[id]) {
                auto* pool = static_cast<ComponentPool<Component>*>(componentPools[id].get());

                if (pool->contains(e)) {
                    pool->erase(e);

                    // If no more of this component type exists, update system availability
                    if (pool->empty()) {
                        std::cout << "[Registry] Last component of type removed, updating system availability\n";
                        availableComponents.reset(id);
                        updateSystemAvailability();
                    }
                }
            }
        }

        /**
        * @brief Checks if entity `e` has a component of type `Component`.
        * @tparam Component Component type to check.
        * @param e Target entity.
        * @return True if the entity has the component, false otherwise.
        */
        template<typename Component>
        bool has(Entity e) const {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();
            
            if (id >= componentPools.size() || !componentPools[id])
                return false;
                    
            auto* pool = static_cast<ComponentPool<Component>*>(componentPools[id].get());
            return pool->contains(e);
        }

        /**
        * @brief Gets a reference to a component of type `Component` for entity `e`.
        * @tparam Component Component type.
        * @param e Target entity.
        * @return Reference to the component.
        */
        template<typename Component>
        Component& get(Entity e) {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();
            auto* pool = static_cast<ComponentPool<Component>*>(componentPools[id].get());
            return pool->get(e);
        }

        /**
        * @brief Const version of get().
        */
        template<typename Component>
        const Component& get(Entity e) const {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();
            auto* pool = static_cast<ComponentPool<Component>*>(componentPools[id].get());
            return pool->get(e);
        }

        /**
        * @brief Returns a reference to the SparseSet storing all components of type `Component`.
        */
        template<typename Component>
        SparseSet<Entity, Component>& view() {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();
            return assurePool<Component>(id);
        }

        /**
        * @brief Iterates through all entities that have all the specified components.
        * @tparam Components Component types to include.
        * @tparam Func Callable with signature `void(Entity, Components&...)`.
        * @param func Function to execute for each matching entity.
        */
        template<typename... Components, typename Func>
        void each(Func&& func) {
            IComponentPool* smallestPool = nullptr;
            size_t smallestSize = SIZE_MAX;

            // Find the smallest pool to optimize iteration
            ((void)[&] {
                ComponentID id = ComponentRegistry::instance().getOrCreateID<Components>();
                assurePool<Components>(id);
                
                if (id < componentPools.size() && componentPools[id]) {
                    IComponentPool* pool = componentPools[id].get();
                    if (pool->size() < smallestSize) {
                        smallestSize = pool->size();
                        smallestPool = pool;
                    }
                }
            }(), ...);
            
            if (!smallestPool) return;

            // Iterate and call function for entities that have all components
            for (size_t i = 0; i < smallestPool->size(); ++i) {
                Entity e = smallestPool->getEntityAt(i);
                if ((has<Components>(e) && ...)) {
                    func(e, get<Components>(e)...);
                }
            }
        }

        /**
        * @brief Adds a system of type `SystemType` to the registry.
        * @tparam SystemType The type of system to add.
        * @param priority Execution order priority (lower = earlier).
        * @param args Arguments forwarded to the system's constructor.
        * @return Reference to the added system.
        */
        template<typename SystemType, typename... Args>
        SystemType& addSystem(int priority = 0, Args&&... args) {
            auto system = std::make_unique<SystemType>(std::forward<Args>(args)...);
            system->priority = priority;
            
            SystemID id = nextSystemID++;
            system->setSystemID(id);
            system->setName(typeid(SystemType).name());
            
            auto* ptr = system.get();
            systems.push_back(std::move(system));
            sortSystems();
            
            std::cout << "[Registry] System added: " << ptr->getName() << "\n";
            updateSystemAvailability();
            
            return *ptr;
        }

        /**
        * @brief Adds a dynamically created system (type-erased).
        */
        void addSystemDynamic(std::unique_ptr<ISystem> system, int priority = 0) {
            system->priority = priority;
            SystemID id = nextSystemID++;
            system->setSystemID(id);
            
            systems.push_back(std::move(system));
            sortSystems();
            
            std::cout << "[Registry] Dynamic system added\n";
            updateSystemAvailability();
        }

        /**
        * @brief Removes a system of the given type.
        */
        template<typename SystemType>
        void removeSystem() {
            systems.erase(
                std::remove_if(systems.begin(), systems.end(),
                    [](const auto& sys) {
                        return dynamic_cast<SystemType*>(sys.get()) != nullptr;
                    }),
                systems.end()
            );
        }

        /**
        * @brief Updates all active systems based on the simulation clock.
        * @param realDt Real-world delta time (in seconds).
        */
        void update(float realDt) {
            int steps = gameClock.update(realDt);

            // Fixed time-step updates for deterministic simulation
            for (int i = 0; i < steps; i++) {
                float fixedDt = gameClock.getFixedDeltaTime();
                
                for (auto& system : systems) {
                    system->update(*this, fixedDt);
                }
            }
        }

        /// @brief Returns a const reference to the internal game clock.
        const GameEngine::GameClock& getClock() const { return gameClock; }

        /// @brief Returns a mutable reference to the internal game clock.
        GameEngine::GameClock& getClock() { return gameClock; }

        /**
        * @brief Updates which systems are active based on available component types.
        */
        void updateSystemAvailability() {
            std::cout << "[Registry] Updating system availability...\n";
            std::cout << "  Available component types: " << availableComponents.count() << "\n";

            for (auto& system : systems) {
                const auto& required = system->getSignature();
                bool wasAvailable = system->hasRequiredComponents;

                // A system is active only if all its required components exist
                system->hasRequiredComponents = (required & availableComponents) == required;
                
                if (wasAvailable != system->hasRequiredComponents) {
                    std::cout << "  System '" << system->getName() 
                            << "' is now " << (system->hasRequiredComponents ? "ACTIVE" : "INACTIVE") 
                            << "\n";
                }
            }
        }

        /**
        * @brief Preallocates space for entities.
        * @param capacity Number of entities to reserve.
        */
        void reserve(size_t capacity) {
            entityManager.reserve(capacity);
        }

        /**
        * @brief Clears all entities, components, and systems.
        */
        void clear() {
            componentPools.clear();
            componentPools.reserve(MAX_COMPONENTS);
            entityManager.clear();
            availableComponents.reset();
            updateSystemAvailability();
        }

        /**
        * @brief Returns the number of currently alive entities.
        */
        size_t alive() const {
            return entityManager.alive();
        }

        /**
        * @brief Returns a bitset of all available component types.
        */
        const ComponentSignature& getAvailableComponents() const {
            return availableComponents;
        }

        /**
        * @brief Counts how many components of type `Component` currently exist.
        */
        template<typename Component>
        size_t count() const {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();
            if (id >= componentPools.size() || !componentPools[id])
                return 0;
            return componentPools[id]->size();
        }

    private:
        GameEngine::GameClock gameClock; ///< Manages real-time and fixed-step updates.

        /** @brief Abstract base for all component pools. */
        struct IComponentPool {
            virtual ~IComponentPool() = default;
            virtual void remove(Entity e) = 0;
            virtual size_t size() const = 0;
            virtual Entity getEntityAt(size_t index) const = 0;
        };

        /** 
        * @brief Templated component pool for a specific component type.
        * Uses SparseSet for efficient storage and lookup.
        */
        template<typename Component>
        struct ComponentPool : IComponentPool {
            SparseSet<Entity, Component> storage;
            
            void remove(Entity e) override { storage.erase(e); }
            size_t size() const override { return storage.size(); }
            Entity getEntityAt(size_t index) const override { return storage.begin()[index]; }

            template<typename... Args>
            Component& emplace(Entity e, Args&&... args) {
                return storage.emplace(e, std::forward<Args>(args)...);
            }

            void erase(Entity e) { storage.erase(e); }
            bool contains(Entity e) const { return storage.contains(e); }
            Component& get(Entity e) { return storage.get(e); }
            const Component& get(Entity e) const { return storage.get(e); }
            bool empty() const { return storage.empty(); }

            auto begin() { return storage.begin(); }
            auto end() { return storage.end(); }
            auto begin() const { return storage.begin(); }
            auto end() const { return storage.end(); }
        };

        /**
        * @brief Ensures that a component pool exists for the given component ID.
        * Creates it if necessary.
        */
        template<typename Component>
        SparseSet<Entity, Component>& assurePool(ComponentID id) {
            if (id >= componentPools.size()) {
                componentPools.resize(id + 1);
            }
            
            if (!componentPools[id]) {
                componentPools[id] = std::make_unique<ComponentPool<Component>>();
            }

            return static_cast<ComponentPool<Component>*>(componentPools[id].get())->storage;
        }

        /// @brief Sorts systems by priority (ascending order).
        void sortSystems() {
            std::sort(systems.begin(), systems.end(),
                [](const auto& a, const auto& b) {
                    return a->priority < b->priority;
                });
        }

        EntityManager entityManager; ///< Handles entity creation and destruction.
        std::vector<std::unique_ptr<IComponentPool>> componentPools; ///< Storage pools for all component types.
        std::vector<std::unique_ptr<ISystem>> systems; ///< All registered systems.
        ComponentSignature availableComponents; ///< Bitset tracking which component types exist.
        SystemID nextSystemID = 0; ///< Counter for assigning unique system IDs.
};