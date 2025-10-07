#pragma once

#include "Types.hpp"
#include <vector>
#include <typeindex>
#include <string>

class ComponentRegistry {
    public:
        static ComponentRegistry& instance() {
            static ComponentRegistry inst;
            return inst;
        }

        template<typename T>
        ComponentID getOrCreateID() {
            static ComponentID cachedID = computeID<T>();
            return cachedID;
        }

        ComponentID getOrCreateID(const std::string& name) {
            for (size_t i = 0; i < nameToID.size(); ++i) {
                if (nameToID[i].first == name) {
                    return nameToID[i].second;
                }
            }
            
            ComponentID id = nextID++;
            nameToID.push_back({name, id});
            if (id >= idToName.size()) {
                idToName.resize(id + 1);
            }
            idToName[id] = name;
            return id;
        }

        const std::string& getName(ComponentID id) const {
            static std::string empty;
            if (id < idToName.size()) {
                return idToName[id];
            }
            return empty;
        }

        ComponentID getID(const std::string& name) const {
            for (size_t i = 0; i < nameToID.size(); ++i) {
                if (nameToID[i].first == name) {
                    return nameToID[i].second;
                }
            }
            return INVALID_ID;
        }

        static constexpr ComponentID INVALID_ID = static_cast<ComponentID>(-1);

    private:
        ComponentRegistry() {
            typeToID.reserve(64);
            nameToID.reserve(64);
            idToName.reserve(64);
        }

        template<typename T>
        ComponentID computeID() {
            auto typeIdx = std::type_index(typeid(T));
        
            for (size_t i = 0; i < typeToID.size(); ++i) {
                if (typeToID[i].first == typeIdx) {
                    return typeToID[i].second;
                }
            }
            
            ComponentID id = nextID++;
            typeToID.push_back({typeIdx, id});
            if (id >= idToName.size()) {
                idToName.resize(id + 1);
            }
            idToName[id] = typeid(T).name();
            return id;
        }
        
        ComponentID nextID = 0;

        std::vector<std::pair<std::type_index, ComponentID>> typeToID;
        std::vector<std::pair<std::string, ComponentID>> nameToID;
        std::vector<std::string> idToName;
};