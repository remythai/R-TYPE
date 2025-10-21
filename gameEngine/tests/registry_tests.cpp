#include <gtest/gtest.h>
#include "../ecs/Registry.hpp"
#include <chrono>
#include <algorithm>

// Composants de test
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
    Health(int hp = 100) : hp(hp) {}
};

struct Tag {
    std::string name;
    Tag(const std::string& n = "") : name(n) {}
};

struct Renderable {
    int layer;
    Renderable(int l = 0) : layer(l) {}
};

// Système de test simple avec CRTP
class MovementSystem : public System<MovementSystem> {
public:
    int updateCount = 0;
    
    MovementSystem() {
        requireComponents<Position>();
        requireComponents<Velocity>();
    }
    
    void onUpdate(Registry& registry, float dt) {
        updateCount++;
        registry.each<Position, Velocity>([dt](auto entity, Position& pos, Velocity& vel) {
            pos.x += vel.vx * dt;
            pos.y += vel.vy * dt;
            pos.z += vel.vz * dt;
        });
    }
};

class HealthSystem : public System<HealthSystem> {
public:
    int updateCount = 0;
    
    HealthSystem() {
        requireComponents<Health>();
    }
    
    void onUpdate(Registry& registry, float dt) {
        updateCount++;
    }
};

// --- Tests de création/destruction d'entités ---

TEST(RegistryTest, CreateEntity) {
    Registry registry;
    auto e = registry.create();
    
    EXPECT_NE(e, EntityManager::INVALID_ENTITY);
    EXPECT_EQ(registry.alive(), 1);
}

TEST(RegistryTest, CreateMultipleEntities) {
    Registry registry;
    auto e1 = registry.create();
    auto e2 = registry.create();
    auto e3 = registry.create();
    
    EXPECT_NE(e1, e2);
    EXPECT_NE(e2, e3);
    EXPECT_NE(e1, e3);
    EXPECT_EQ(registry.alive(), 3);
}

TEST(RegistryTest, DestroyEntity) {
    Registry registry;
    auto e = registry.create();
    
    EXPECT_EQ(registry.alive(), 1);
    registry.destroy(e);
    EXPECT_EQ(registry.alive(), 0);
}

TEST(RegistryTest, DestroyEntityWithComponents) {
    Registry registry;
    auto e = registry.create();
    registry.emplace<Position>(e, 1.0f, 2.0f, 3.0f);
    registry.emplace<Velocity>(e, 0.5f, 0.5f, 0.5f);
    
    EXPECT_TRUE(registry.has<Position>(e));
    EXPECT_TRUE(registry.has<Velocity>(e));
    
    registry.destroy(e);
    EXPECT_EQ(registry.alive(), 0);
}

// --- Tests de composants ---

TEST(RegistryTest, EmplaceComponent) {
    Registry registry;
    auto e = registry.create();
    
    auto& pos = registry.emplace<Position>(e, 10.0f, 20.0f, 30.0f);
    
    EXPECT_EQ(pos.x, 10.0f);
    EXPECT_EQ(pos.y, 20.0f);
    EXPECT_EQ(pos.z, 30.0f);
}

TEST(RegistryTest, HasComponent) {
    Registry registry;
    auto e = registry.create();
    
    EXPECT_FALSE(registry.has<Position>(e));
    
    registry.emplace<Position>(e);
    EXPECT_TRUE(registry.has<Position>(e));
}

TEST(RegistryTest, GetComponent) {
    Registry registry;
    auto e = registry.create();
    registry.emplace<Position>(e, 5.0f, 10.0f, 15.0f);
    
    auto& pos = registry.get<Position>(e);
    EXPECT_EQ(pos.x, 5.0f);
    EXPECT_EQ(pos.y, 10.0f);
    EXPECT_EQ(pos.z, 15.0f);
}

TEST(RegistryTest, ModifyComponent) {
    Registry registry;
    auto e = registry.create();
    registry.emplace<Position>(e, 1.0f, 2.0f, 3.0f);
    
    auto& pos = registry.get<Position>(e);
    pos.x = 100.0f;
    
    EXPECT_EQ(registry.get<Position>(e).x, 100.0f);
}

TEST(RegistryTest, RemoveComponent) {
    Registry registry;
    auto e = registry.create();
    registry.emplace<Position>(e);
    
    EXPECT_TRUE(registry.has<Position>(e));
    
    registry.remove<Position>(e);
    EXPECT_FALSE(registry.has<Position>(e));
}

TEST(RegistryTest, MultipleComponentsPerEntity) {
    Registry registry;
    auto e = registry.create();
    
    registry.emplace<Position>(e, 1.0f, 2.0f, 3.0f);
    registry.emplace<Velocity>(e, 0.1f, 0.2f, 0.3f);
    registry.emplace<Health>(e, 100);
    
    EXPECT_TRUE(registry.has<Position>(e));
    EXPECT_TRUE(registry.has<Velocity>(e));
    EXPECT_TRUE(registry.has<Health>(e));
    EXPECT_FALSE(registry.has<Tag>(e));
}

TEST(RegistryTest, CountComponents) {
    Registry registry;
    
    EXPECT_EQ(registry.count<Position>(), 0);
    
    auto e1 = registry.create();
    auto e2 = registry.create();
    auto e3 = registry.create();
    
    registry.emplace<Position>(e1);
    registry.emplace<Position>(e2);
    
    EXPECT_EQ(registry.count<Position>(), 2);
    
    registry.emplace<Position>(e3);
    EXPECT_EQ(registry.count<Position>(), 3);
    
    registry.remove<Position>(e1);
    EXPECT_EQ(registry.count<Position>(), 2);
}

// --- Tests de view ---

TEST(RegistryTest, ViewSingleComponent) {
    Registry registry;
    auto e1 = registry.create();
    auto e2 = registry.create();
    
    registry.emplace<Position>(e1, 1.0f, 2.0f, 3.0f);
    registry.emplace<Position>(e2, 4.0f, 5.0f, 6.0f);
    
    auto& view = registry.view<Position>();
    EXPECT_EQ(view.size(), 2);
}

// --- Tests de each ---

TEST(RegistryTest, EachSingleComponent) {
    Registry registry;
    auto e1 = registry.create();
    auto e2 = registry.create();
    
    registry.emplace<Position>(e1, 1.0f, 2.0f, 3.0f);
    registry.emplace<Position>(e2, 4.0f, 5.0f, 6.0f);
    
    int count = 0;
    registry.each<Position>([&count](auto e, Position& pos) {
        count++;
        pos.x += 10.0f;
    });
    
    EXPECT_EQ(count, 2);
    EXPECT_EQ(registry.get<Position>(e1).x, 11.0f);
    EXPECT_EQ(registry.get<Position>(e2).x, 14.0f);
}

TEST(RegistryTest, EachMultipleComponents) {
    Registry registry;
    auto e1 = registry.create();
    auto e2 = registry.create();
    auto e3 = registry.create();
    
    registry.emplace<Position>(e1);
    registry.emplace<Velocity>(e1);
    
    registry.emplace<Position>(e2);
    
    registry.emplace<Position>(e3);
    registry.emplace<Velocity>(e3);
    
    int count = 0;
    registry.each<Position, Velocity>([&count](auto e, Position& pos, Velocity& vel) {
        count++;
    });
    
    EXPECT_EQ(count, 2); // Seulement e1 et e3 ont les deux composants
}

TEST(RegistryTest, EachWithNoMatchingEntities) {
    Registry registry;
    auto e = registry.create();
    registry.emplace<Position>(e);
    
    int count = 0;
    registry.each<Position, Velocity>([&count](auto e, Position& pos, Velocity& vel) {
        count++;
    });
    
    EXPECT_EQ(count, 0);
}

// --- Tests de systèmes ---

TEST(RegistryTest, AddSystem) {
    Registry registry;
    auto& system = registry.addSystem<MovementSystem>();
    
    EXPECT_EQ(system.updateCount, 0);
}

TEST(RegistryTest, SystemUpdateCalled) {
    Registry registry;
    auto& system = registry.addSystem<MovementSystem>();
    
    // Créer une entité avec les composants requis
    auto e = registry.create();
    registry.emplace<Position>(e, 0.0f, 0.0f, 0.0f);
    registry.emplace<Velocity>(e, 1.0f, 1.0f, 1.0f);
    
    registry.update(0.016f);
    
    EXPECT_GT(system.updateCount, 0);
}

TEST(RegistryTest, SystemPriority) {
    Registry registry;
    
    auto& sys1 = registry.addSystem<MovementSystem>(10);
    auto& sys2 = registry.addSystem<HealthSystem>(5);
    
    EXPECT_EQ(sys1.priority, 10);
    EXPECT_EQ(sys2.priority, 5);
}

TEST(RegistryTest, SystemAvailabilityWithoutComponents) {
    Registry registry;
    auto& system = registry.addSystem<MovementSystem>();
    
    // Le système nécessite Position et Velocity, mais aucun n'existe
    EXPECT_FALSE(system.hasRequiredComponents);
}

TEST(RegistryTest, SystemAvailabilityWithComponents) {
    Registry registry;
    auto& system = registry.addSystem<MovementSystem>();
    
    auto e = registry.create();
    registry.emplace<Position>(e);
    
    // Toujours pas actif, il manque Velocity
    EXPECT_FALSE(system.hasRequiredComponents);
    
    registry.emplace<Velocity>(e);
    
    // Maintenant actif
    EXPECT_TRUE(system.hasRequiredComponents);
}


TEST(RegistryTest, RemoveSystem) {
    Registry registry;
    registry.addSystem<MovementSystem>();
    
    registry.removeSystem<MovementSystem>();
    
    // Pas de crash lors de l'update
    registry.update(0.016f);
}

// --- Tests de clear ---

TEST(RegistryTest, ClearEntities) {
    Registry registry;
    
    for (int i = 0; i < 10; ++i) {
        auto e = registry.create();
        registry.emplace<Position>(e);
    }
    
    EXPECT_EQ(registry.alive(), 10);
    EXPECT_EQ(registry.count<Position>(), 10);
    
    registry.clear();
    
    EXPECT_EQ(registry.alive(), 0);
    EXPECT_EQ(registry.count<Position>(), 0);
}

// --- Tests de reserve ---

TEST(RegistryTest, ReserveDoesNotCreateEntities) {
    Registry registry;
    registry.reserve(1000);
    
    EXPECT_EQ(registry.alive(), 0);
}

// --- Tests de cas limites ---

TEST(RegistryTest, RemoveNonExistentComponent) {
    Registry registry;
    auto e = registry.create();
    
    // Ne devrait pas crasher
    registry.remove<Position>(e);
    EXPECT_FALSE(registry.has<Position>(e));
}

TEST(RegistryTest, GetNonExistentComponentThrows) {
    Registry registry;
    auto e = registry.create();
    
    // Comportement : peut throw ou undefined behavior selon SparseSet
    // À adapter selon votre implémentation
}

TEST(RegistryTest, MultipleSystemsWithSameType) {
    Registry registry;
    
    auto& sys1 = registry.addSystem<MovementSystem>(1);
    auto& sys2 = registry.addSystem<MovementSystem>(2);
    
    EXPECT_NE(&sys1, &sys2);
}

// --- Tests d'intégration ---

TEST(RegistryTest, CompleteWorkflow) {
    Registry registry;
    auto& system = registry.addSystem<MovementSystem>();
    
    // Créer des entités avec composants
    auto e1 = registry.create();
    registry.emplace<Position>(e1, 0.0f, 0.0f, 0.0f);
    registry.emplace<Velocity>(e1, 1.0f, 0.0f, 0.0f);
    
    auto e2 = registry.create();
    registry.emplace<Position>(e2, 10.0f, 0.0f, 0.0f);
    registry.emplace<Velocity>(e2, -1.0f, 0.0f, 0.0f);
    
    EXPECT_TRUE(system.hasRequiredComponents);
    
    // Simuler un update
    registry.update(1.0f);
    
    EXPECT_GT(system.updateCount, 0);
    
    // Vérifier que les positions ont changé
    // Note: dépend de votre GameClock et du fixed timestep
}

TEST(RegistryTest, EntityReuseAfterDestroy) {
    Registry registry;
    
    auto e1 = registry.create();
    registry.emplace<Position>(e1);
    
    registry.destroy(e1);
    
    auto e2 = registry.create();
    EXPECT_EQ(e1, e2); // ID réutilisé
    
    // Le nouveau composant ne doit pas exister
    EXPECT_FALSE(registry.has<Position>(e2));
}

// --- Tests de performance ---

TEST(RegistryPerformance, CreateDestroyManyEntities) {
    Registry registry;
    const size_t COUNT = 100000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<Registry::Entity> entities;
    entities.reserve(COUNT);
    
    for (size_t i = 0; i < COUNT; ++i) {
        entities.push_back(registry.create());
    }
    
    for (auto e : entities) {
        registry.destroy(e);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 500);
    EXPECT_EQ(registry.alive(), 0);
}

TEST(RegistryPerformance, EmplaceComponentsPerformance) {
    Registry registry;
    const size_t COUNT = 50000;
    
    std::vector<Registry::Entity> entities;
    entities.reserve(COUNT);
    
    for (size_t i = 0; i < COUNT; ++i) {
        entities.push_back(registry.create());
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (auto e : entities) {
        registry.emplace<Position>(e, 1.0f, 2.0f, 3.0f);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 200);
    EXPECT_EQ(registry.count<Position>(), COUNT);
}

TEST(RegistryPerformance, EachIterationSpeed) {
    Registry registry;
    const size_t COUNT = 100000;
    
    for (size_t i = 0; i < COUNT; ++i) {
        auto e = registry.create();
        registry.emplace<Position>(e, 0.0f, 0.0f, 0.0f);
        registry.emplace<Velocity>(e, 1.0f, 1.0f, 1.0f);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    registry.each<Position, Velocity>([](auto e, Position& pos, Velocity& vel) {
        pos.x += vel.vx;
        pos.y += vel.vy;
        pos.z += vel.vz;
    });
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 100);
}

TEST(RegistryPerformance, MixedComponentIteration) {
    Registry registry;
    const size_t COUNT = 10000;
    
    // Créer des entités avec différentes combinaisons de composants
    for (size_t i = 0; i < COUNT; ++i) {
        auto e = registry.create();
        registry.emplace<Position>(e);
        
        if (i % 2 == 0) {
            registry.emplace<Velocity>(e);
        }
        
        if (i % 3 == 0) {
            registry.emplace<Health>(e);
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int count = 0;
    registry.each<Position, Velocity>([&count](auto e, Position& pos, Velocity& vel) {
        count++;
    });
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(count, COUNT / 2); // Seulement les entités paires
    EXPECT_LT(duration.count(), 50);
}

TEST(RegistryPerformance, FragmentationHandling) {
    Registry registry;
    std::vector<Registry::Entity> entities;
    
    // Créer 10000 entités
    for (int i = 0; i < 10000; ++i) {
        auto e = registry.create();
        entities.push_back(e);
        registry.emplace<Position>(e);
    }
    
    // Détruire les entités paires
    for (size_t i = 0; i < entities.size(); i += 2) {
        registry.destroy(entities[i]);
    }
    
    EXPECT_EQ(registry.alive(), 5000);
    EXPECT_EQ(registry.count<Position>(), 5000);
    
    // Recréer 5000 entités
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 5000; ++i) {
        auto e = registry.create();
        registry.emplace<Position>(e);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(registry.alive(), 10000);
    EXPECT_LT(duration.count(), 100);
}

TEST(RegistryPerformance, SystemUpdateOverhead) {
    Registry registry;
    
    // Ajouter plusieurs systèmes et garder leurs références
    std::vector<MovementSystem*> systems;
    for (int i = 0; i < 10; ++i) {
        systems.push_back(&registry.addSystem<MovementSystem>(i));
    }
    
    // Créer des entités avec les composants requis
    for (int i = 0; i < 1000; ++i) {
        auto e = registry.create();
        registry.emplace<Position>(e, 0.0f, 0.0f, 0.0f);
        registry.emplace<Velocity>(e, 1.0f, 1.0f, 1.0f);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    const int UPDATE_COUNT = 100;
    for (int i = 0; i < UPDATE_COUNT; ++i) {
        registry.update(0.016f);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Vérifier que tous les systèmes ont bien été appelés
    // Note: le nombre exact dépend du fixed timestep du GameClock
    for (auto* sys : systems) {
        EXPECT_GT(sys->updateCount, 0);
    }
    
    // 10 systèmes × ~100 updates × 1000 entités
    // Avec fixed timestep, le nombre réel d'updates peut varier
    EXPECT_LT(duration.count(), 1000);
}