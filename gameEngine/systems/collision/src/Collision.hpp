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

/**
 * @class MotionSystem
 * @brief System responsible for detecting collisions and applying damage between entities.
 *
 * This system implements a simple **grid-based spatial partitioning** for collision detection.
 * Entities are divided into grid cells based on their positions and collider sizes.
 * Collisions are checked within each cell and neighboring cells.
 *
 * Entities must have the following components:
 * - `Position` — entity position in 2D space.
 * - `Renderable` — provides screen dimensions.
 * - `Collider` — hitbox dimensions.
 * - `Damage` — damage value dealt on collision.
 * - `Health` — current hit points, reduced on collision.
 */
class MotionSystem : public System<MotionSystem> {
private:
    /**
     * @brief Checks and resolves a collision between two entities.
     *
     * If their colliders overlap, applies damage to each entity accordingly,
     * ensuring `Health.currentHp` does not drop below zero.
     *
     * @param e1 First entity.
     * @param e2 Second entity.
     * @param registry ECS registry for accessing components.
     */
    void collide(uint32_t e1, uint32_t e2, Registry& registry) {
        Position e1Pos = registry.get<Position>(e1);
        Position e2Pos = registry.get<Position>(e2);
        Collider e1Hitbox = registry.get<Collider>(e1);
        Collider e2Hitbox = registry.get<Collider>(e2);

        Damage e1Damage = registry.get<Damage>(e1);
        Damage e2Damage = registry.get<Damage>(e2);

        Health e1Health = registry.get<Health>(e1);
        Health e2Health = registry.get<Health>(e2);

        // Axis-Aligned Bounding Box (AABB) collision check
        if (e1Pos.x < e2Pos.x + e2Hitbox.size.x &&
            e1Pos.x + e1Hitbox.size.x > e2Pos.x &&
            e1Pos.y < e2Pos.y + e2Hitbox.size.y &&
            e1Pos.y + e1Hitbox.size.y > e2Pos.y) {

            // Apply damage safely, preventing negative HP
            e1Health.currentHp = e1Health.currentHp > 0 ? std::max(e1Health.currentHp - e2Damage.dmg, 0) : e1Health.currentHp;
            e2Health.currentHp = e2Health.currentHp > 0 ? std::max(e2Health.currentHp - e1Damage.dmg, 0) : e2Health.currentHp;
        }
    }

public:
    /**
     * @brief Constructs the MotionSystem and declares its required components.
     *
     * Requires:
     * - `Position`, `Renderable`, `Collider`, `Damage`, `Health`
     */
    MotionSystem() {
        requireComponents<Position, Renderable, Collider, Damage, Health>();
    }

    /**
     * @brief Updates the system and processes collisions for all entities.
     *
     * Implements a grid-based spatial partitioning:
     * - Divides the screen into grid cells of size `hitboxSizeMean`.
     * - Each entity is assigned to cells based on its position and collider.
     * - Collisions are checked within the cell and neighboring cells.
     *
     * @param registry The ECS registry managing entities and components.
     * @param dt Delta time since the last update (unused here, but kept for consistency).
     */
    void onUpdate(Registry& registry, float dt) {
        updateCount++;

        Renderable screenInfo = registry.get<Renderable>(0);
        uint32_t screenSizeX = screenInfo.screenSizeX;
        uint32_t screenSizeY = screenInfo.screenSizeY;
        uint32_t hitboxSizeMean = 64;

        size_t gridWidth = screenSizeX / hitboxSizeMean;
        size_t gridHeight = screenSizeY / hitboxSizeMean;

        // 3D grid: grid[x][y] contains all entity IDs in that cell
        std::vector<std::vector<std::vector<uint32_t>>> grid(
            gridWidth, std::vector<std::vector<uint32_t>>(gridHeight));

        // Populate grid with entities based on their positions and colliders
        registry.each<Position, Renderable, Collider>(
            [dt, &grid, hitboxSizeMean](auto e, Position& pos, Renderable& render, Collider& collider) {
                float rightPos = pos.x + collider.size.x;
                float topPos = pos.y + collider.size.y;

                int leftCell = pos.x / hitboxSizeMean;
                int bottomCell = pos.y / hitboxSizeMean;
                int rightCell = rightPos / hitboxSizeMean;
                int topCell = topPos / hitboxSizeMean;

                for (size_t i = leftCell; i <= rightCell; i++) {
                    for (size_t j = bottomCell; j <= topCell; j++) {
                        grid[i][j].push_back(e);
                    }
                }
            }
        );

        // Check collisions within cells and neighboring cells
        for (size_t i = 0; i < grid.size(); i++) {
            for (size_t j = 0; j < grid[i].size(); j++) {
                while (grid[i][j].size() > 1) {
                    for (size_t k = 1; k < grid[i][j].size(); k++) {
                        collide(grid[i][j][0], grid[i][j][k], registry);
                    }
                    // Neighboring cells
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
                        for (size_t k = 0; k < grid[i + 1][j + 1].size(); k++) {
                            collide(grid[i][j][0], grid[i + 1][j + 1][k], registry);
                        }
                    }
                    if (i > 0 && j + 1 < grid[i].size()) {
                        for (size_t k = 0; k < grid[i - 1][j + 1].size(); k++) {
                            collide(grid[i][j][0], grid[i - 1][j + 1][k], registry);
                        }
                    }
                    grid[i][j].erase(grid[i][j].begin());
                }
            }
        }
    }

    /// @brief Tracks how many times the system has updated (useful for profiling or debugging).
    int updateCount = 0;
};
}  // namespace GameEngine
