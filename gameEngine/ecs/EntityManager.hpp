#pragma once

#include <vector>
#include <cstdint>

/**
 * @class EntityManager
 * @brief Handles creation and destruction of entities within the ECS framework.
 *
 * The EntityManager is responsible for generating unique entity IDs,
 * reusing destroyed entity IDs for efficiency, and keeping track of
 * how many entities are currently alive.
 *
 * Entities are represented by simple unsigned integers (uint32_t).
 */
class EntityManager {
    public:
        /// @brief Type used to represent an entity.
        using Entity = uint32_t;

        /// @brief Special invalid entity identifier (used as a null handle).
        static constexpr Entity INVALID_ENTITY = static_cast<Entity>(-1);

        /**
        * @brief Creates a new entity.
        *
        * If there are recycled entity IDs available in the free list,
        * one of those will be reused. Otherwise, a new unique ID is generated.
        *
        * @return The newly created entity ID.
        */
        Entity create() {
            // Reuse IDs from entities that were destroyed, if available
            if (!freeList.empty()) {
                Entity e = freeList.back();
                freeList.pop_back();
                aliveCount++;
                return e;
            }

            // Otherwise, assign a new unique ID
            aliveCount++;
            return nextEntity++;
        }

        /**
        * @brief Destroys an entity, making its ID available for reuse.
        *
        * This does not automatically remove components from the registry;
        * it simply marks the entity ID as reusable.
        *
        * @param e Entity to destroy.
        */
        void destroy(Entity e) {
            freeList.push_back(e);
            aliveCount--;
        }

        /**
        * @brief Returns the number of currently active (alive) entities.
        * @return Number of active entities.
        */
        size_t alive() const {
            return aliveCount;
        }

        /**
        * @brief Reserves memory for a given number of entities in the free list.
        * 
        * This does not create entities; it just preallocates memory to avoid reallocations.
        * @param capacity Expected number of entities to reserve for.
        */
        void reserve(size_t capacity) {
            freeList.reserve(capacity);
        }

        /**
        * @brief Clears all entity data, resetting the manager to its initial state.
        *
        * All entities are invalidated, and counters are reset.
        * Useful when restarting a simulation or game.
        */
        void clear() {
            nextEntity = 0;
            aliveCount = 0;
            freeList.clear();
        }

    private:
        Entity nextEntity = 0;              ///< Next entity ID to assign when creating a new entity.
        size_t aliveCount = 0;              ///< Number of currently active entities.
        std::vector<Entity> freeList;       ///< Pool of destroyed entities ready for reuse.
};