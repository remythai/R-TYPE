#pragma once

#include "Types.hpp"
#include <vector>
#include <typeindex>
#include <string>

/**
 * @class ComponentRegistry
 * @brief Global registry that maps component types and names to unique IDs.
 *
 * This class assigns each component type a unique integer ID (`ComponentID`),
 * used by the ECS framework to efficiently identify and manage component pools.
 *
 * It supports both:
 *  - **Type-based registration** (via C++ templates)
 *  - **String-based registration** (for dynamic/runtime use)
 *
 * The registry follows the Singleton pattern — only one instance exists globally.
 */
class ComponentRegistry {
    public:
        /**
        * @brief Provides access to the global ComponentRegistry instance.
        * @return Reference to the singleton instance.
        */
        static ComponentRegistry& instance() {
            static ComponentRegistry inst;
            return inst;
        }

        /**
        * @brief Retrieves or creates a unique ID for the given component type `T`.
        *
        * The ID is generated once per type and cached for all future calls.
        * 
        * @tparam T Component type.
        * @return The unique ComponentID for the type `T`.
        */
        template<typename T>
        ComponentID getOrCreateID() {
            // Each component type caches its ID statically for speed
            static ComponentID cachedID = computeID<T>();
            return cachedID;
        }

        /**
        * @brief Retrieves or creates a unique component ID from a string name.
        *
        * Useful when working with dynamically loaded components (e.g. from scripts or configuration files).
        *
        * @param name Component name (e.g., "Transform", "RigidBody").
        * @return Unique ComponentID associated with this name.
        */
        ComponentID getOrCreateID(const std::string& name) {
            // Check if the name already has an assigned ID
            for (size_t i = 0; i < nameToID.size(); ++i) {
                if (nameToID[i].first == name) {
                    return nameToID[i].second;
                }
            }
            
            // Create a new entry
            ComponentID id = nextID++;
            nameToID.push_back({name, id});

            // Resize name table if needed
            if (id >= idToName.size()) {
                idToName.resize(id + 1);
            }
            idToName[id] = name;

            return id;
        }

        /**
        * @brief Returns the registered name of a component given its ID.
        * 
        * @param id Component ID to look up.
        * @return Reference to the component name string, or an empty string if not found.
        */
        const std::string& getName(ComponentID id) const {
            static std::string empty;
            if (id < idToName.size()) {
                return idToName[id];
            }
            return empty;
        }

        /**
        * @brief Returns the component ID associated with a given name.
        * 
        * @param name Component name.
        * @return The ComponentID if found, otherwise INVALID_ID.
        */
        ComponentID getID(const std::string& name) const {
            for (size_t i = 0; i < nameToID.size(); ++i) {
                if (nameToID[i].first == name) {
                    return nameToID[i].second;
                }
            }
            return INVALID_ID;
        }

        /// @brief Constant used to represent an invalid or uninitialized component ID.
        static constexpr ComponentID INVALID_ID = static_cast<ComponentID>(-1);

    private:
        /**
        * @brief Private constructor for singleton pattern.
        * Initializes internal containers with reserved capacity.
        */
        ComponentRegistry() {
            typeToID.reserve(64);
            nameToID.reserve(64);
            idToName.reserve(64);
        }

        /**
        * @brief Computes a new unique ID for a type `T`, or returns the existing one.
        *
        * This is called only once per component type.
        * It uses `std::type_index` to identify unique C++ types at runtime.
        *
        * @tparam T Component type.
        * @return Unique ComponentID for the type.
        */
        template<typename T>
        ComponentID computeID() {
            auto typeIdx = std::type_index(typeid(T));
        
            // Search if this type is already registered
            for (size_t i = 0; i < typeToID.size(); ++i) {
                if (typeToID[i].first == typeIdx) {
                    return typeToID[i].second;
                }
            }
            
            // Otherwise, assign a new ID
            ComponentID id = nextID++;
            typeToID.push_back({typeIdx, id});

            // Store type name for debugging or serialization
            if (id >= idToName.size()) {
                idToName.resize(id + 1);
            }
            idToName[id] = typeid(T).name();

            return id;
        }
        
        ComponentID nextID = 0; ///< Next available unique component ID counter.

        /// @brief Maps C++ type_index → ComponentID (used for static type lookup).
        std::vector<std::pair<std::type_index, ComponentID>> typeToID;

        /// @brief Maps string name → ComponentID (used for dynamic component lookup).
        std::vector<std::pair<std::string, ComponentID>> nameToID;

        /// @brief Reverse lookup: maps ComponentID → string name.
        std::vector<std::string> idToName;
};