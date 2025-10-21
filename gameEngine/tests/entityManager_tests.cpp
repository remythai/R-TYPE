
#include <gtest/gtest.h>
#include "../ecs/EntityManager.hpp"
#include <algorithm>

TEST(EntityManagerTest, CreateDestroySequence) {
    EntityManager manager;
    std::vector<EntityManager::Entity> entities;

    for (int i = 0; i < 10; ++i)
        entities.push_back(manager.create());

    EXPECT_EQ(manager.alive(), 10);

    for (auto e : entities)
        manager.destroy(e);

    EXPECT_EQ(manager.alive(), 0);

    auto newEntity = manager.create();
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), newEntity) != entities.end());
}

TEST(EntityManagerTest, CreateIncrementsAliveCount) {
    EntityManager manager;
    auto e1 = manager.create();
    auto e2 = manager.create();

    EXPECT_EQ(manager.alive(), 2);
    EXPECT_NE(e1, e2);
}

TEST(EntityManagerTest, DestroyDecrementsAliveCount) {
    EntityManager manager;
    auto e = manager.create();
    EXPECT_EQ(manager.alive(), 1);

    manager.destroy(e);
    EXPECT_EQ(manager.alive(), 0);
}

TEST(EntityManagerTest, ReusesDestroyedIds) {
    EntityManager manager;
    auto e1 = manager.create();
    manager.destroy(e1);
    auto e2 = manager.create();

    EXPECT_EQ(e1, e2);
}

TEST(EntityManagerTest, ClearResetsState) {
    EntityManager manager;
    for (int i = 0; i < 5; ++i) manager.create();
    EXPECT_EQ(manager.alive(), 5);

    manager.clear();
    EXPECT_EQ(manager.alive(), 0);

    auto e = manager.create();
    EXPECT_EQ(e, 0u);
}

TEST(EntityManagerTest, ReserveDoesNotCreateEntities) {
    EntityManager manager;
    manager.reserve(100);
    EXPECT_EQ(manager.alive(), 0);
}

TEST(EntityManagerTest, CreateDestroySequenceChain) {
    EntityManager manager;
    std::vector<EntityManager::Entity> entities;

    for (int i = 0; i < 10; ++i)
        entities.push_back(manager.create());

    EXPECT_EQ(manager.alive(), 10);

    for (auto e : entities)
        manager.destroy(e);

    EXPECT_EQ(manager.alive(), 0);

    auto newEntity = manager.create();
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), newEntity) != entities.end());
}

TEST(EntityManagerTest, InvalidEntityConstant) {
    EXPECT_EQ(EntityManager::INVALID_ENTITY, static_cast<uint32_t>(-1));
}

TEST(EntityManagerTest, ReusesIdsInLIFOOrder) {
    EntityManager manager;
    auto e1 = manager.create();
    auto e2 = manager.create();
    auto e3 = manager.create();
    
    manager.destroy(e1);
    manager.destroy(e2);
    
    auto e4 = manager.create();
    EXPECT_EQ(e4, e2);
    
    auto e5 = manager.create();
    EXPECT_EQ(e5, e1);
}

TEST(EntityManagerTest, DoubleDestroyBehavior) {
    EntityManager manager;
    auto e = manager.create();
    
    manager.destroy(e);
    EXPECT_EQ(manager.alive(), 0);

    manager.destroy(e);
    EXPECT_EQ(manager.alive(), static_cast<size_t>(-1));
}

TEST(EntityManagerTest, HandlesLargeNumberOfEntities) {
    EntityManager manager;
    const size_t COUNT = 10000;
    std::vector<EntityManager::Entity> entities;
    entities.reserve(COUNT);
    
    for (size_t i = 0; i < COUNT; ++i) {
        entities.push_back(manager.create());
    }
    
    EXPECT_EQ(manager.alive(), COUNT);
    EXPECT_EQ(entities.back(), COUNT - 1);
}

#include <chrono>

TEST(EntityManagerPerformance, CreateDestroyBenchmark) {
    EntityManager manager;
    const size_t ITERATIONS = 1000000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < ITERATIONS; ++i) {
        auto e = manager.create();
        manager.destroy(e);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_LT(duration.count(), 1000);
}

TEST(EntityManagerPerformance, FragmentationTest) {
    EntityManager manager;
    std::vector<EntityManager::Entity> entities;

    for (int i = 0; i < 10000; ++i) {
        entities.push_back(manager.create());
    }

    for (size_t i = 0; i < entities.size(); i += 2) {
        manager.destroy(entities[i]);
    }
    
    EXPECT_EQ(manager.alive(), 5000);

    std::vector<EntityManager::Entity> newEntities;
    for (int i = 0; i < 5000; ++i) {
        newEntities.push_back(manager.create());
    }

    for (auto ne : newEntities) {
        EXPECT_LT(ne, 10000u);
    }
}