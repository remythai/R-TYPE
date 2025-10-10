#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#include "../../../Registry.hpp"
#include "../../../System.hpp"
#include "../../../components/collider/src/Collider.hpp"
#include "../../../components/position/src/Position.hpp"
#include "../../../components/renderable/src/Renderable.hpp"
#include "../../../components/damage/src/Damage.hpp"
#include "../../../components/health/src/Health.hpp"

namespace GameEngine {
class MotionSystem : public System<MotionSystem>
{
   private:
    void collide(uint32_t e1, uint32_t e2, Registry& registry) {
        GameEngine::Position e1Pos = registry.get<GameEngine::Position>(e1);
        GameEngine::Position e2Pos = registry.get<GameEngine::Position>(e2);
        GameEngine::Collider e1Hitbox = registry.get<GameEngine::Collider>(e1);
        GameEngine::Collider e2Hitbox = registry.get<GameEngine::Collider>(e2);

        GameEngine::Damage e1Damage = registry.get<GameEngine::Damage>(e1);
        GameEngine::Damage e2Damage = registry.get<GameEngine::Damage>(e2);

        GameEngine::Health e1Health = registry.get<GameEngine::Health>(e1);
        GameEngine::Health e2Health = registry.get<GameEngine::Health>(e2);

        if (e1Pos.x < e2Pos.x + e2Hitbox.size.x && e1Pos.x + e1Hitbox.size.x > e2Pos.x && e1Pos.y < e2Pos.y + e2Hitbox.size.y && e1Pos.y + e1Hitbox.size.y > e2Pos.y) {
            e1Health.currentHp = e1Health.currentHp > 0 ? std::max(e1Health.currentHp - e2Damage.dmg, 0) : e1Health.currentHp;
            e2Health.currentHp = e2Health.currentHp > 0 ? std::max(e2Health.currentHp - e1Damage.dmg, 0) : e2Health.currentHp;
        }
    }
   public:
    MotionSystem()
    {
        requireComponents<
            GameEngine::Position, GameEngine::Renderable,
            GameEngine::Collider, GameEngine::Damage, GameEngine::Health>();
    }

    void onUpdate(Registry& registry, float dt)
    {
        updateCount++;

        GameEngine::Renderable screenInfo =
            registry.get<GameEngine::Renderable>(0);
        uint32_t screenSizeX = screenInfo.screenSizeX;
        uint32_t screenSizeY = screenInfo.screenSizeY;
        uint32_t hitboxSizeMean = 64;

        size_t gridWidth = screenSizeX / hitboxSizeMean;
        size_t gridHeight = screenSizeY / hitboxSizeMean;

        std::vector<std::vector<std::vector<uint32_t>>> grid(
            gridWidth, std::vector<std::vector<uint32_t>>(gridHeight));

        registry.each<Position, Renderable, Collider>([dt, &grid, hitboxSizeMean](auto e, Position& pos, Renderable& render, Collider& collider) {
            float rightPos = pos.x + collider.size.x;
            float topPos = pos.y + collider.size.y;

            int leftCell = pos.x / hitboxSizeMean;
            int bottomCell = pos.y / hitboxSizeMean;
            int rightCell = rightPos / hitboxSizeMean;
            int topCell = topPos / hitboxSizeMean;

            for (size_t i = leftCell; i <= rightCell; i++) {
                for (size_t j = bottomCell; i <= topCell; i++) {
                    grid[i][j].push_back(e);
                }
            }
        });
        for (size_t i = 0; i < grid.size(); i++) {
            for (size_t j = 0; j < grid[i].size(); j++) {
                while (grid[i][j].size() > 1) {
                    for (size_t k = 1; k < grid[i][j].size(); k++) {
                        collide(grid[i][j][0], grid[i][j][k], registry);
                    }
                    if (i + 1 < grid.size()) {
                        for (size_t k = 0; k < grid[i + 1][j].size(); k++) {
                            collide(grid[i][j][0], grid[i + 1][j][k], registry);
                        }
                    }
                    if (j + 1 < grid[i].size()) {
                        for (size_t k = 0; k < grid[i][j + 1].size(); k++) {
                            collide(grid[i][j][0], grid[i][j + 1][k], registry);
                        }
                    }
                    if (i + 1 < grid.size() && j + 1 < grid[i].size()) {
                        for (size_t k = 0; k < grid[i + 1][j + 1].size();
                             k++) {
                            collide(grid[i][j][0], grid[i + 1][j + 1][k], registry);
                        }
                    }
                    if (i - 1 >= 0 && j + 1 < grid[i].size()) {
                        for (size_t k = 0; k < grid[i - 1][j + 1].size(); k++) {
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
