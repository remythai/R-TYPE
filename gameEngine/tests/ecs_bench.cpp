#include <benchmark/benchmark.h>
#include "../ecs/Registry.hpp"
#include "../ecs/EntityManager.hpp"

struct Position {
    float x, y, z;
    Position(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

struct Velocity {
    float vx, vy, vz;
    Velocity(float vx = 0, float vy = 0, float vz = 0) : vx(vx), vy(vy), vz(vz) {}
};

struct Health {
    int hp;
    explicit Health(int hp = 100) : hp(hp) {}
};

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
    ->Args({10'000})
    ->Args({100'000})
    ->Args({1'000'000})
    ->Args({10'000'000});

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
    ->Args({10'000})
    ->Args({100'000})
    ->Args({1'000'000})
    ->Args({10'000'000});

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
    ->Args({10'000})
    ->Args({100'000})
    ->Args({1'000'000})
    ->Args({10'000'000});

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
    ->Args({10'000})
    ->Args({100'000})
    ->Args({1'000'000})
    ->Args({10'000'000});

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
                registry.emplace<Health>(e, 100);
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
    ->Args({10'000})
    ->Args({100'000})
    ->Args({1'000'000})
    ->Args({10'000'000});

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
    ->Args({10'000})
    ->Args({100'000})
    ->Args({1'000'000})
    ->Args({10'000'000});

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
    ->Args({10'000})
    ->Args({100'000})
    ->Args({1'000'000})
    ->Args({10'000'000});

BENCHMARK_MAIN();

