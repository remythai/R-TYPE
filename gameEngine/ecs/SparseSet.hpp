#pragma once

#include <vector>

template<typename Entity, typename Component>
class SparseSet {
    public:
        bool contains(Entity e) const {
            return (e < sparse.size() && sparse[e] != npos);
        }

        Component& get(Entity e) { 
            return data[sparse[e]]; 
        }

        const Component& get(Entity e) const { 
            return data[sparse[e]]; 
        }

        template<typename... Args>
        void emplace(Entity e, Args&&... args) {
            if (e >= sparse.size()) {
                sparse.resize(e + 1, npos);
            }
            if (!contains(e)) {
                sparse[e] = dense.size();
                dense.push_back(e);
                data.emplace_back(std::forward<Args>(args)...);
            }
        }

        void erase(Entity e) {
            if (!contains(e))
                return;
            size_t idx = sparse[e];
            size_t last = dense.size() - 1;

            Entity movedEntity = dense[last];
            dense[idx] = movedEntity;
            data[idx] = std::move(data[last]);
            sparse[movedEntity] = idx;

            dense.pop_back();
            data.pop_back();
            sparse[e] = npos;
        }

        void reserve(size_t capacity) {
            dense.reserve(capacity);
            data.reserve(capacity);
        }

        void clear() {
            sparse.clear();
            dense.clear();
            data.clear();
        }

        size_t size() const { 
            return dense.size(); 
        }

        bool empty() const { 
            return dense.empty(); 
        }

        auto begin() { return dense.begin(); }
        auto end()   { return dense.end(); }
        auto begin() const { return dense.begin(); }
        auto end()   const { return dense.end(); }

        std::vector<Component>& components() { return data; }
        const std::vector<Component>& components() const { return data; }

    private:
        static constexpr size_t npos = static_cast<size_t>(-1);

        std::vector<size_t> sparse;
        std::vector<Entity> dense;
        std::vector<Component> data;
};
