#include <benchmark/benchmark.h>
#include <vector>
#include <bitset>

#include "../ecs/Registry.hpp"
#include "../ecs/EntityManager.hpp"

// Compos simplifiés pour certains tests
struct Position {
    float x, y, z;
    Position(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

struct Velocity {
    float vx, vy, vz;
    Velocity(float vx = 0, float vy = 0, float vz = 0) : vx(vx), vy(vy), vz(vz) {}
};

struct HealthSimple {
    int hp;
    explicit HealthSimple(int hp = 100) : hp(hp) {}
};

// Compos réels du moteur
#include "../components/position/src/Position.hpp"
#include "../components/collider/src/Collider.hpp"
#include "../components/damage/src/Damage.hpp"
#include "../components/health/src/Health.hpp"
#include "../components/renderable/src/Renderable.hpp"
#include "../systems/collision/src/Collision.hpp"

// -----------------------------------------------------------------------------
// EntityManager create/destroy
// -----------------------------------------------------------------------------
static void BM_EntityManager_CreateDestroy(benchmark::State& state) {
    const std::size_t ITERATIONS = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        EntityManager manager;
        for (std::size_t i = 0; i < ITERATIONS; ++i) {
            auto e = manager.create();
            manager.destroy(e);
        }
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_EntityManager_CreateDestroy)
    ->Args({1'000})
    ->Args({5'000})
    ->Args({10'000})
    ->Args({50'000})
    ->Args({100'000})
    ->Args({500'000})
    ->Args({1'000'000})
    ->Args({5'000'000})
    ->Args({10'000'000});

// -----------------------------------------------------------------------------
// Registry create/destroy
// -----------------------------------------------------------------------------
static void BM_Registry_CreateDestroyManyEntities(benchmark::State& state) {
    const std::size_t COUNT = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        Registry registry;
        std::vector<Registry::Entity> entities;
        entities.reserve(COUNT);
        for (std::size_t i = 0; i < COUNT; ++i) {
            entities.push_back(registry.create());
        }
        for (auto e : entities) {
            registry.destroy(e);
        }
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Registry_CreateDestroyManyEntities)
    ->Args({1'000})
    ->Args({5'000})
    ->Args({10'000})
    ->Args({50'000})
    ->Args({100'000})
    ->Args({500'000})
    ->Args({1'000'000})
    ->Args({5'000'000})
    ->Args({10'000'000});

// -----------------------------------------------------------------------------
// Emplace Position (simple ECS test)
// -----------------------------------------------------------------------------
static void BM_Registry_EmplacePosition(benchmark::State& state) {
    const std::size_t COUNT = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        Registry registry;
        std::vector<Registry::Entity> entities;
        entities.reserve(COUNT);
        for (std::size_t i = 0; i < COUNT; ++i) {
            entities.push_back(registry.create());
        }
        for (auto e : entities) {
            registry.emplace<Position>(e, 1.0f, 2.0f, 3.0f);
        }
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Registry_EmplacePosition)
    ->Args({1'000})
    ->Args({5'000})
    ->Args({10'000})
    ->Args({50'000})
    ->Args({100'000})
    ->Args({500'000})
    ->Args({1'000'000})
    ->Args({5'000'000})
    ->Args({10'000'000});

// -----------------------------------------------------------------------------
// each Position + Velocity
// -----------------------------------------------------------------------------
static void BM_Registry_Each_PositionVelocity(benchmark::State& state) {
    const std::size_t COUNT = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        Registry registry;
        for (std::size_t i = 0; i < COUNT; ++i) {
            auto e = registry.create();
            registry.emplace<Position>(e, 0.0f, 0.0f, 0.0f);
            registry.emplace<Velocity>(e, 1.0f, 1.0f, 1.0f);
        }
        registry.each<Position, Velocity>([](auto, Position& pos, Velocity& vel) {
            pos.x += vel.vx;
            pos.y += vel.vy;
            pos.z += vel.vz;
        });
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Registry_Each_PositionVelocity)
    ->Args({1'000})
    ->Args({5'000})
    ->Args({10'000})
    ->Args({50'000})
    ->Args({100'000})
    ->Args({500'000})
    ->Args({1'000'000})
    ->Args({5'000'000})
    ->Args({10'000'000});

// -----------------------------------------------------------------------------
// Mixed components iteration
// -----------------------------------------------------------------------------
static void BM_Registry_MixedComponentIteration(benchmark::State& state) {
    const std::size_t COUNT = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        Registry registry;
        for (std::size_t i = 0; i < COUNT; ++i) {
            auto e = registry.create();
            registry.emplace<Position>(e);
            if (i % 2 == 0) {
                registry.emplace<Velocity>(e);
            }
            if (i % 3 == 0) {
                registry.emplace<HealthSimple>(e, 100);
            }
        }
        int count = 0;
        registry.each<Position, Velocity>([&](auto, Position&, Velocity&) {
            ++count;
        });
        benchmark::DoNotOptimize(count);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Registry_MixedComponentIteration)
    ->Args({1'000})
    ->Args({5'000})
    ->Args({10'000})
    ->Args({50'000})
    ->Args({100'000})
    ->Args({500'000})
    ->Args({1'000'000})
    ->Args({5'000'000})
    ->Args({10'000'000});

// -----------------------------------------------------------------------------
// Fragmentation handling (Registry)
// -----------------------------------------------------------------------------
static void BM_Registry_FragmentationHandling(benchmark::State& state) {
    const std::size_t COUNT = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        Registry registry;
        std::vector<Registry::Entity> entities;
        for (std::size_t i = 0; i < COUNT; ++i) {
            auto e = registry.create();
            entities.push_back(e);
            registry.emplace<Position>(e);
        }
        for (std::size_t i = 0; i < entities.size(); i += 2) {
            registry.destroy(entities[i]);
        }
        for (std::size_t i = 0; i < COUNT / 2; ++i) {
            auto e = registry.create();
            registry.emplace<Position>(e);
        }
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Registry_FragmentationHandling)
    ->Args({1'000})
    ->Args({5'000})
    ->Args({10'000})
    ->Args({50'000})
    ->Args({100'000})
    ->Args({500'000})
    ->Args({1'000'000})
    ->Args({5'000'000})
    ->Args({10'000'000});

// -----------------------------------------------------------------------------
// Fragmentation handling (EntityManager)
// -----------------------------------------------------------------------------
static void BM_EntityManager_Fragmentation(benchmark::State& state) {
    const std::size_t COUNT = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        EntityManager manager;
        std::vector<EntityManager::Entity> entities;
        entities.reserve(COUNT);
        for (std::size_t i = 0; i < COUNT; ++i) {
            entities.push_back(manager.create());
        }
        for (std::size_t i = 0; i < entities.size(); i += 2) {
            manager.destroy(entities[i]);
        }
        std::vector<EntityManager::Entity> newEntities;
        newEntities.reserve(COUNT / 2);
        for (std::size_t i = 0; i < COUNT / 2; ++i) {
            newEntities.push_back(manager.create());
        }
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_EntityManager_Fragmentation)
    ->Args({1'000})
    ->Args({5'000})
    ->Args({10'000})
    ->Args({50'000})
    ->Args({100'000})
    ->Args({500'000})
    ->Args({1'000'000})
    ->Args({5'000'000})
    ->Args({10'000'000});

// -----------------------------------------------------------------------------
// Collision system benchmark - distribution "normale"
// -----------------------------------------------------------------------------
static void BM_CollisionSystem_Update(benchmark::State& state) {
    const std::size_t COUNT = static_cast<std::size_t>(state.range(0));

    for (auto _ : state) {
        Registry registry;

        for (std::size_t i = 0; i < COUNT; ++i) {
            auto e = registry.create();

            // Position en grille sur l'écran 1920x1080
            float x = static_cast<float>((i % 200) * 20);   // 0..4000
            float y = static_cast<float>((i / 200) * 20);   // étalé en Y
            registry.emplace<GameEngine::Position>(e, x, y);

            // Collider : hitbox 64x64, collision avec tout
            std::bitset<8> selector(0xFF);
            std::bitset<8> diff(0xFF);
            GameEngine::Collider col(
                vec2(0.f, 0.f),
                selector,
                diff,
                vec2(64.f, 64.f)
            );
            registry.emplace<GameEngine::Collider>(e, col);

            // Damage
            registry.emplace<GameEngine::Damage>(e, 10);

            // Health
            registry.emplace<GameEngine::Health>(e, 100.f, 100.f);

            // Renderable par défaut
            registry.emplace<GameEngine::Renderable>(e);
        }

        GameEngine::Collision collisionSystem;
        collisionSystem.onUpdate(registry, 0.016f);

        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_CollisionSystem_Update)
    ->Args({1'000})
    ->Args({5'000})
    ->Args({10'000})
    ->Args({50'000})
    ->Args({100'000})
    ->Args({500'000})
    ->Args({1'000'000})
    ->Args({5'000'000})
    ->Args({10'000'000});

// -----------------------------------------------------------------------------
// Collision system benchmark - cas dense (beaucoup de collisions)
// -----------------------------------------------------------------------------
static void BM_CollisionSystem_Update_Dense(benchmark::State& state) {
    const std::size_t COUNT = static_cast<std::size_t>(state.range(0));

    for (auto _ : state) {
        Registry registry;

        for (std::size_t i = 0; i < COUNT; ++i) {
            auto e = registry.create();

            // Toutes les entités dans une zone très réduite
            float x = 500.f + static_cast<float>(i % 8);
            float y = 500.f + static_cast<float>((i / 8) % 8);
            registry.emplace<GameEngine::Position>(e, x, y);

            std::bitset<8> selector(0xFF);
            std::bitset<8> diff(0xFF);
            GameEngine::Collider col(
                vec2(0.f, 0.f),
                selector,
                diff,
                vec2(64.f, 64.f)
            );
            registry.emplace<GameEngine::Collider>(e, col);

            registry.emplace<GameEngine::Damage>(e, 10);
            registry.emplace<GameEngine::Health>(e, 100.f, 100.f);
            registry.emplace<GameEngine::Renderable>(e);
        }

        GameEngine::Collision collisionSystem;
        collisionSystem.onUpdate(registry, 0.016f);

        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_CollisionSystem_Update_Dense)
    ->Args({1'000})
    ->Args({5'000})
    ->Args({10'000});

BENCHMARK_MAIN();
