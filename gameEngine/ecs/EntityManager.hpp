#pragma once

#include <vector>
#include <cstdint>

class EntityManager {
    public:
        using Entity = uint32_t;

        Entity create() {
            if (!freeList.empty()) {
                Entity e = freeList.back();
                freeList.pop_back();
                aliveCount++;
                return e;
            }
            aliveCount++;
            return nextEntity++;
        }

        void destroy(Entity e) {
            freeList.push_back(e);
            aliveCount--;
        }

        size_t alive() const {
            return aliveCount;
        }

        void reserve(size_t capacity) {
            freeList.reserve(capacity);
        }

        void clear() {
            nextEntity = 0;
            aliveCount = 0;
            freeList.clear();
        }

    private:
        Entity nextEntity = 0;
        size_t aliveCount = 0;
        std::vector<Entity> freeList;
};