#pragma once

#include "Types.hpp"
#include "ComponentRegistry.hpp"
#include <string>
#include <vector>

class Registry;

class ISystem {
    public:
        virtual ~ISystem() = default;
        virtual void update(Registry& registry, float dt) = 0;
        virtual const ComponentSignature& getSignature() const = 0;
        virtual SystemID getSystemID() const = 0;
        virtual const std::string& getName() const = 0;
        virtual void setSystemID(SystemID id) = 0;
        
        int priority = 0;
        bool enabled = true;
        bool hasRequiredComponents = false;
};

template<typename Derived>
class System : public ISystem {
    public:
        void update(Registry& registry, float dt) override {
            if (enabled && hasRequiredComponents) {
                static_cast<Derived*>(this)->onUpdate(registry, dt);
            }
        }

        const ComponentSignature& getSignature() const override {
            return signature;
        }

        SystemID getSystemID() const override {
            return systemID;
        }

        const std::string& getName() const override {
            return name;
        }

        void setSystemID(SystemID id) override {
            systemID = id;
        }

        void setName(const std::string& n) {
            name = n;
        }

    protected:
        template<typename... Components>
        void requireComponents() {
            signature.reset();
            (signature.set(ComponentRegistry::instance().getOrCreateID<Components>()), ...);
        }

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
        ComponentSignature signature;
        SystemID systemID = 0;
        std::string name;

        friend class Registry;
};