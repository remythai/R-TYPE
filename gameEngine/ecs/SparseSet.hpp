#pragma once

#include <vector>

/**
 * @class SparseSet
 * @brief Efficient associative container mapping entities to components.
 *
 * The `SparseSet` is a core ECS data structure used for storing components.
 * It provides O(1) insertion, lookup, and removal of components by entity ID.
 *
 * Internally, it maintains three parallel arrays:
 *  - **sparse**: maps entity IDs to indices in the dense array
 *  - **dense**: stores the active entity IDs (contiguously)
 *  - **data**: stores the components associated with entities
 *
 * This layout ensures tight memory packing and cache-friendly iteration.
 *
 * @tparam Entity The entity identifier type (e.g. `uint32_t`).
 * @tparam Component The component type to store (e.g. `Transform`,
 * `RigidBody`).
 */
template <typename Entity, typename Component>
class SparseSet
{
   public:
    /**
     * @brief Checks if a given entity currently has an associated component.
     * @param e The entity to check.
     * @return True if the entity has a component stored, false otherwise.
     */
    bool contains(Entity e) const
    {
        return (e < sparse.size() && sparse[e] != npos);
    }

    /**
     * @brief Retrieves the component associated with an entity.
     * @param e The entity whose component to retrieve.
     * @return Reference to the component.
     * @warning Undefined behavior if the entity does not have this component.
     */
    Component& get(Entity e)
    {
        return data[sparse[e]];
    }

    /**
     * @brief Const version of get().
     */
    const Component& get(Entity e) const
    {
        return data[sparse[e]];
    }

    /**
     * @brief Adds or constructs a component for the given entity.
     *
     * If the entity does not already have a component, a new one is emplaced
     * in tightly packed arrays, maintaining spatial locality for iteration.
     *
     * @tparam Args Constructor argument types for `Component`.
     * @param e The target entity.
     * @param args Arguments forwarded to the component constructor.
     */
    template <typename... Args>
    void emplace(Entity e, Args&&... args)
    {
        // Resize sparse vector if needed to accommodate entity ID
        if (e >= sparse.size()) {
            sparse.resize(e + 1, npos);
        }

        // Only add if the entity doesn't already have this component
        if (!contains(e)) {
            sparse[e] = dense.size();
            dense.push_back(e);
            data.emplace_back(std::forward<Args>(args)...);
        }
    }

    /**
     * @brief Removes the component associated with an entity.
     *
     * Maintains array compactness by moving the last element into the erased
     * position.
     *
     * @param e Entity whose component to remove.
     */
    void erase(Entity e)
    {
        if (!contains(e))
            return;

        size_t idx = sparse[e];
        size_t last = dense.size() - 1;

        // Move the last element to the erased spot for contiguous storage
        Entity movedEntity = dense[last];
        dense[idx] = movedEntity;
        data[idx] = std::move(data[last]);
        sparse[movedEntity] = idx;

        // Pop the last (now moved) element
        dense.pop_back();
        data.pop_back();
        sparse[e] = npos;
    }

    /**
     * @brief Reserves memory for the given number of components.
     *
     * Useful to avoid reallocations when adding many components at once.
     * @param capacity Number of elements to reserve for.
     */
    void reserve(size_t capacity)
    {
        dense.reserve(capacity);
        data.reserve(capacity);
    }

    /**
     * @brief Clears all stored components and associated entity mappings.
     */
    void clear()
    {
        sparse.clear();
        dense.clear();
        data.clear();
    }

    /**
     * @brief Returns the number of active components.
     * @return Number of components currently stored.
     */
    size_t size() const
    {
        return dense.size();
    }

    /**
     * @brief Checks if the set is empty (no components).
     * @return True if empty, false otherwise.
     */
    bool empty() const
    {
        return dense.empty();
    }

    /// @brief Iterator access (begin) — iterates over entity IDs.
    auto begin()
    {
        return dense.begin();
    }

    /// @brief Iterator access (end) — iterates over entity IDs.
    auto end()
    {
        return dense.end();
    }

    /// @brief Const iterator access (begin).
    auto begin() const
    {
        return dense.begin();
    }

    /// @brief Const iterator access (end).
    auto end() const
    {
        return dense.end();
    }

    /**
     * @brief Returns a reference to the internal component vector.
     * @return Reference to the vector of components.
     */
    std::vector<Component>& components()
    {
        return data;
    }

    /**
     * @brief Const version of components().
     */
    const std::vector<Component>& components() const
    {
        return data;
    }

   private:
    /// @brief Sentinel value indicating that an entity has no component entry.
    static constexpr size_t npos = static_cast<size_t>(-1);

    /// @brief Maps entity IDs → indices in the dense array.
    std::vector<size_t> sparse;

    /// @brief Stores all entity IDs that currently have this component type.
    std::vector<Entity> dense;

    /// @brief Stores component data in the same order as `dense` for fast
    /// iteration.
    std::vector<Component> data;
};
