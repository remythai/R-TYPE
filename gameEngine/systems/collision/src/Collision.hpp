#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"
#include "../../../components/collider/src/Collider.hpp"
#include "../../../components/position/src/Position.hpp"
#include "../../../components/renderable/src/Renderable.hpp"
#include "../../../components/damage/src/Damage.hpp"
#include "../../../components/health/src/Health.hpp"

namespace GameEngine {
class Collision : public System<Collision>
{
   private:
    void collide(uint32_t e1, uint32_t e2, Registry& registry) {
        if (e1 == e2)
            return;

        GameEngine::Position e1Pos = registry.get<GameEngine::Position>(e1);
        GameEngine::Position e2Pos = registry.get<GameEngine::Position>(e2);

        GameEngine::Collider e1Collider = registry.get<GameEngine::Collider>(e1);
        GameEngine::Collider e2Collider = registry.get<GameEngine::Collider>(e2);

        GameEngine::Damage e1Damage = registry.get<GameEngine::Damage>(e1);
        GameEngine::Damage e2Damage = registry.get<GameEngine::Damage>(e2);

        GameEngine::Health &e1Health = registry.get<GameEngine::Health>(e1);
        GameEngine::Health &e2Health = registry.get<GameEngine::Health>(e2);

        vec2 e1HitboxPos = e1Pos.pos + e1Collider.originTranslation;
        vec2 e2HitboxPos = e2Pos.pos + e2Collider.originTranslation;

        if ((e1Collider.entitySelector & e2Collider.entitySelector).any() &&
            e1HitboxPos.x < e2HitboxPos.x + e2Collider.size.x &&
            e1HitboxPos.x + e1Collider.size.x > e2HitboxPos.x &&
            e1HitboxPos.y < e2HitboxPos.y + e2Collider.size.y &&
            e1HitboxPos.y + e1Collider.size.y > e2HitboxPos.y) {
            e1Health.currentHp = e1Health.currentHp > 0 ? std::max(e1Health.currentHp - e2Damage.dmg, 0) : e1Health.currentHp;
            e2Health.currentHp = e2Health.currentHp > 0 ? std::max(e2Health.currentHp - e1Damage.dmg, 0) : e2Health.currentHp;
        }
    }
   public:
    Collision()
    {
        requireComponents<
            GameEngine::Position, GameEngine::Renderable,
            GameEngine::Collider, GameEngine::Damage, GameEngine::Health>();
    }

    void onUpdate(Registry& registry, float dt)
    {
        updateCount++;

        uint32_t screenSizeX = 1920;
        uint32_t screenSizeY = 1080;
        uint32_t hitboxSizeMean = 64;

        int gridWidth = screenSizeX / hitboxSizeMean + 1;
        int gridHeight = screenSizeY / hitboxSizeMean + 1;

        std::vector<std::vector<std::vector<uint32_t>>> grid(
            gridWidth, std::vector<std::vector<uint32_t>>(gridHeight));

        registry.each<Position, Renderable, Collider, Damage, Health>([dt, &grid, hitboxSizeMean](auto e, Position& pos, Renderable& render, Collider& collider, Damage &damage, Health& health) {
            if (pos.pos.x < 0 || pos.pos.y < 0)
                return;
            float rightPos = pos.pos.x + collider.size.x;
            float topPos = pos.pos.y + collider.size.y;

            int leftCell = pos.pos.x / hitboxSizeMean;
            int bottomCell = pos.pos.y / hitboxSizeMean;
            int rightCell = rightPos / hitboxSizeMean;
            int topCell = topPos / hitboxSizeMean;

            for (int i = leftCell; i <= rightCell; i++) {
                for (int j = bottomCell; j <= topCell; j++) {
                    grid[i][j].push_back(e);
                }
            }
        });
        for (int i = 0; i < grid.size(); i++) {
            for (int j = 0; j < grid[i].size(); j++) {
                while (grid[i][j].size() > 1) {
                    for (int k = 1; k < grid[i][j].size(); k++) {
                        collide(grid[i][j][0], grid[i][j][k], registry);
                    }
                    if (i + 1 < grid.size()) {
                        for (int k = 0; k < grid[i + 1][j].size(); k++) {
                            collide(grid[i][j][0], grid[i + 1][j][k], registry);
                        }
                    }
                    if (j + 1 < grid[i].size()) {
                        for (int k = 0; k < grid[i][j + 1].size(); k++) {
                            collide(grid[i][j][0], grid[i][j + 1][k], registry);
                        }
                    }
                    if (i + 1 < grid.size() && j + 1 < grid[i].size()) {
                        for (int k = 0; k < grid[i + 1][j + 1].size();
                             k++) {
                            collide(grid[i][j][0], grid[i + 1][j + 1][k], registry);
                        }
                    }
                    if (i - 1 >= 0 && j + 1 < grid[i - 1].size()) {
                        for (int k = 0; k < grid[i - 1][j + 1].size(); k++) {
                            collide(grid[i][j][0], grid[i - 1][j + 1][k], registry);
                        }
                    }
                    grid[i][j].erase(grid[i][j].begin());
                }
            }
        }
    }
    int updateCount = 0;
};
}  // namespace GameEngine
