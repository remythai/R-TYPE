#pragma once

#include <vector>
#include <stdexcept>

template<typename Entity, typename Component>
class SparseSet {
    public:
        bool contains(Entity e) const {
            return (e < sparse.size() && sparse[e] != npos);
        }

        Component& get(Entity e) { return data[sparse[e]]; }

        const Component& get(Entity e) const { return data[sparse[e]]; }

        void insert(Entity e, const Component& c) {
            if (e >= sparse.size()) {
                sparse.resize(e + 1, npos);
            }
            if (!contains(e)) {
                sparse[e] = dense.size();
                dense.push_back(e);
                data.push_back(c);
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

        auto begin() { return dense.begin(); }
        auto end()   { return dense.end(); }
        auto begin() const { return dense.begin(); }
        auto end()   const { return dense.end(); }

    private:
        static constexpr size_t npos = static_cast<size_t>(-1);

        std::vector<size_t> sparse;
        std::vector<Entity> dense;
        std::vector<Component> data;
};
