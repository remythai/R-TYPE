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

class Registry {
    public:
        using Entity = EntityManager::Entity;

        Registry() {
            componentPools.reserve(MAX_COMPONENTS);
            systems.reserve(32);
            updateSystemAvailability();
        }

        Entity create() {
            return entityManager.create();
        }

        void destroy(Entity e) {
            for (auto& pool : componentPools) {
                if (pool) {
                    pool->remove(e);
                }
            }
            entityManager.destroy(e);
        }

        template<typename Component, typename... Args>
        Component& emplace(Entity e, Args&&... args) {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();

            auto& pool = assurePool<Component>(id);

            bool wasEmpty = pool.size() == 0;

            pool.emplace(e, std::forward<Args>(args)...);

            if (wasEmpty) {
                std::cout << "[Registry] First component of type added, updating system availability\n";
                availableComponents.set(id);
                updateSystemAvailability();
            }
            
            return pool.get(e);
        }

        template<typename Component>
        void remove(Entity e) {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();
            
            if (id < componentPools.size() && componentPools[id]) {
                auto* pool = static_cast<ComponentPool<Component>*>(componentPools[id].get());

                if (pool->contains(e)) {
                    pool->erase(e);

                    if (pool->empty()) {
                        std::cout << "[Registry] Last component of type removed, updating system availability\n";
                        availableComponents.reset(id);
                        updateSystemAvailability();
                    }
                }
            }
        }

        template<typename Component>
        bool has(Entity e) const {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();
            
            if (id >= componentPools.size() || !componentPools[id])
                return false;
                
            auto* pool = static_cast<ComponentPool<Component>*>(componentPools[id].get());
            return pool->contains(e);
        }

        template<typename Component>
        Component& get(Entity e) {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();
            auto* pool = static_cast<ComponentPool<Component>*>(componentPools[id].get());
            return pool->get(e);
        }

        template<typename Component>
        const Component& get(Entity e) const {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();
            auto* pool = static_cast<ComponentPool<Component>*>(componentPools[id].get());
            return pool->get(e);
        }

        template<typename Component>
        SparseSet<Entity, Component>& view() {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();
            return assurePool<Component>(id);
        }

        template<typename... Components, typename Func>
        void each(Func&& func) {
            IComponentPool* smallestPool = nullptr;
            size_t smallestSize = SIZE_MAX;
    
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

            for (size_t i = 0; i < smallestPool->size(); ++i) {
                Entity e = smallestPool->getEntityAt(i);

                if ((has<Components>(e) && ...)) {
                    func(e, get<Components>(e)...);
                }
            }
        }

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

        void addSystemDynamic(std::unique_ptr<ISystem> system, int priority = 0) {
            system->priority = priority;
            SystemID id = nextSystemID++;
            system->setSystemID(id);
            
            systems.push_back(std::move(system));
            sortSystems();
            
            std::cout << "[Registry] Dynamic system added\n";
            updateSystemAvailability();
        }

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

        void update(float dt) {
            for (auto& system : systems) {
                system->update(*this, dt);
            }
        }

        void updateSystemAvailability() {
            std::cout << "[Registry] Updating system availability...\n";
            std::cout << "  Available component types: " << availableComponents.count() << "\n";

            for (auto& system : systems) {
                const auto& required = system->getSignature();
                bool wasAvailable = system->hasRequiredComponents;

                system->hasRequiredComponents = (required & availableComponents) == required;
                
                if (wasAvailable != system->hasRequiredComponents) {
                    std::cout << "  System '" << system->getName() 
                            << "' is now " << (system->hasRequiredComponents ? "ACTIVE" : "INACTIVE") 
                            << "\n";
                }
            }
        }

        void reserve(size_t capacity) {
            entityManager.reserve(capacity);
        }

        void clear() {
            componentPools.clear();
            componentPools.reserve(MAX_COMPONENTS);
            entityManager.clear();
            availableComponents.reset();
            updateSystemAvailability();
        }

        size_t alive() const {
            return entityManager.alive();
        }

        const ComponentSignature& getAvailableComponents() const {
            return availableComponents;
        }

        template<typename Component>
        size_t count() const {
            ComponentID id = ComponentRegistry::instance().getOrCreateID<Component>();
            if (id >= componentPools.size() || !componentPools[id])
                return 0;
            return componentPools[id]->size();
        }

    private:
        struct IComponentPool {
            virtual ~IComponentPool() = default;
            virtual void remove(Entity e) = 0;
            virtual size_t size() const = 0;
            virtual Entity getEntityAt(size_t index) const = 0;
        };

        template<typename Component>
        struct ComponentPool : IComponentPool {
            SparseSet<Entity, Component> storage;
            
            void remove(Entity e) override {
                storage.erase(e);
            }
            size_t size() const override {
                return storage.size();
            }
            Entity getEntityAt(size_t index) const override {
                return storage.begin()[index];
            }

            template<typename... Args>
            Component& emplace(Entity e, Args&&... args) {
                return storage.emplace(e, std::forward<Args>(args)...);
            }
            
            void erase(Entity e) {
                storage.erase(e);
            }
            
            bool contains(Entity e) const {
                return storage.contains(e);
            }
            
            Component& get(Entity e) {
                return storage.get(e);
            }
            
            const Component& get(Entity e) const {
                return storage.get(e);
            }
            
            bool empty() const {
                return storage.empty();
            }
            
            auto begin() { return storage.begin(); }
            auto end() { return storage.end(); }
            auto begin() const { return storage.begin(); }
            auto end() const { return storage.end(); }
        };

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

        void sortSystems() {
            std::sort(systems.begin(), systems.end(),
                [](const auto& a, const auto& b) {
                    return a->priority < b->priority;
                });
        }

        EntityManager entityManager;

        std::vector<std::unique_ptr<IComponentPool>> componentPools;

        std::vector<std::unique_ptr<ISystem>> systems;
        
        ComponentSignature availableComponents;
        SystemID nextSystemID = 0;
};