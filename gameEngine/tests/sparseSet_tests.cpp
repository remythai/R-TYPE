#include <gtest/gtest.h>
#include "../ecs/SparseSet.hpp"
#include <string>
#include <chrono>
#include <algorithm>
#include <random>

// ============================================================================
// TEST STRUCTURES
// ============================================================================

struct Position {
    float x, y, z;
    Position(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct Velocity {
    float vx, vy, vz;
    Velocity(float vx = 0, float vy = 0, float vz = 0) : vx(vx), vy(vy), vz(vz) {}
};

struct Name {
    std::string value;
    Name(const std::string& v = "") : value(v) {}
};

struct NonCopyable {
    int value;
    NonCopyable(int v) : value(v) {}
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

using Entity = uint32_t;

// ============================================================================
// BASIC TEST
// ============================================================================

TEST(SparseSetTest, InitiallyEmpty) {
    SparseSet<Entity, Position> set;
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0);
}

TEST(SparseSetTest, EmplaceAddsComponent) {
    SparseSet<Entity, Position> set;
    set.emplace(0, 1.0f, 2.0f, 3.0f);
    
    EXPECT_FALSE(set.empty());
    EXPECT_EQ(set.size(), 1);
    EXPECT_TRUE(set.contains(0));
}

TEST(SparseSetTest, GetReturnsCorrectComponent) {
    SparseSet<Entity, Position> set;
    set.emplace(5, 10.0f, 20.0f, 30.0f);
    
    const Position& pos = set.get(5);
    EXPECT_EQ(pos.x, 10.0f);
    EXPECT_EQ(pos.y, 20.0f);
    EXPECT_EQ(pos.z, 30.0f);
}

TEST(SparseSetTest, GetAllowsModification) {
    SparseSet<Entity, Position> set;
    set.emplace(0, 1.0f, 2.0f, 3.0f);
    
    set.get(0).x = 100.0f;
    EXPECT_EQ(set.get(0).x, 100.0f);
}

TEST(SparseSetTest, ContainsReturnsFalseForNonExistent) {
    SparseSet<Entity, Position> set;
    set.emplace(5, 1.0f, 2.0f, 3.0f);
    
    EXPECT_TRUE(set.contains(5));
    EXPECT_FALSE(set.contains(0));
    EXPECT_FALSE(set.contains(10));
}

// ============================================================================
// DELETE TESTS
// ============================================================================

TEST(SparseSetTest, EraseRemovesComponent) {
    SparseSet<Entity, Position> set;
    set.emplace(0, 1.0f, 2.0f, 3.0f);
    
    EXPECT_TRUE(set.contains(0));
    set.erase(0);
    
    EXPECT_FALSE(set.contains(0));
    EXPECT_EQ(set.size(), 0);
}

TEST(SparseSetTest, EraseNonExistentDoesNothing) {
    SparseSet<Entity, Position> set;
    set.emplace(0, 1.0f, 2.0f, 3.0f);
    
    set.erase(999);
    EXPECT_EQ(set.size(), 1);
    EXPECT_TRUE(set.contains(0));
}

TEST(SparseSetTest, EraseWithSwapMaintainsContiguity) {
    SparseSet<Entity, Position> set;
    set.emplace(0, 0.0f, 0.0f, 0.0f);
    set.emplace(1, 1.0f, 1.0f, 1.0f);
    set.emplace(2, 2.0f, 2.0f, 2.0f);
    set.emplace(3, 3.0f, 3.0f, 3.0f);

    set.erase(1);
    
    EXPECT_EQ(set.size(), 3);
    EXPECT_FALSE(set.contains(1));

    EXPECT_TRUE(set.contains(0));
    EXPECT_TRUE(set.contains(2));
    EXPECT_TRUE(set.contains(3));

    EXPECT_EQ(set.get(0).x, 0.0f);
    EXPECT_EQ(set.get(2).x, 2.0f);
    EXPECT_EQ(set.get(3).x, 3.0f);
}

TEST(SparseSetTest, EraseLastElement) {
    SparseSet<Entity, Position> set;
    set.emplace(0, 1.0f, 2.0f, 3.0f);
    set.emplace(1, 4.0f, 5.0f, 6.0f);
    
    set.erase(1);
    
    EXPECT_EQ(set.size(), 1);
    EXPECT_TRUE(set.contains(0));
    EXPECT_FALSE(set.contains(1));
}

// ============================================================================
// EMPLACE TESTS
// ============================================================================

TEST(SparseSetTest, EmplaceDoesNotDuplicateIfAlreadyPresent) {
    SparseSet<Entity, Position> set;
    set.emplace(0, 1.0f, 2.0f, 3.0f);
    set.emplace(0, 99.0f, 99.0f, 99.0f);
    
    EXPECT_EQ(set.size(), 1);
    EXPECT_EQ(set.get(0).x, 1.0f);
}

TEST(SparseSetTest, EmplaceWithHighEntityId) {
    SparseSet<Entity, Position> set;
    set.emplace(1000, 1.0f, 2.0f, 3.0f);
    
    EXPECT_TRUE(set.contains(1000));
    EXPECT_EQ(set.get(1000).x, 1.0f);
    EXPECT_EQ(set.size(), 1);
}

TEST(SparseSetTest, EmplaceWithSparseIds) {
    SparseSet<Entity, Position> set;
    set.emplace(5, 5.0f, 5.0f, 5.0f);
    set.emplace(100, 100.0f, 100.0f, 100.0f);
    set.emplace(1000, 1000.0f, 1000.0f, 1000.0f);
    
    EXPECT_EQ(set.size(), 3);
    EXPECT_FALSE(set.contains(0));
    EXPECT_FALSE(set.contains(50));
    EXPECT_TRUE(set.contains(5));
    EXPECT_TRUE(set.contains(100));
    EXPECT_TRUE(set.contains(1000));
}

TEST(SparseSetTest, EmplaceWithNonCopyableType) {
    SparseSet<Entity, NonCopyable> set;
    set.emplace(0, 42);
    
    EXPECT_TRUE(set.contains(0));
    EXPECT_EQ(set.get(0).value, 42);
}

TEST(SparseSetTest, EmplaceWithComplexType) {
    SparseSet<Entity, Name> set;
    set.emplace(0, "Player");
    set.emplace(1, "Enemy");
    
    EXPECT_EQ(set.get(0).value, "Player");
    EXPECT_EQ(set.get(1).value, "Enemy");
}

// ============================================================================
// CLEAR AND RESERVE TESTS
// ============================================================================

TEST(SparseSetTest, ClearRemovesAllComponents) {
    SparseSet<Entity, Position> set;
    for (int i = 0; i < 10; ++i) {
        set.emplace(i, i * 1.0f, i * 2.0f, i * 3.0f);
    }
    
    EXPECT_EQ(set.size(), 10);
    set.clear();
    
    EXPECT_EQ(set.size(), 0);
    EXPECT_TRUE(set.empty());
    for (int i = 0; i < 10; ++i) {
        EXPECT_FALSE(set.contains(i));
    }
}

TEST(SparseSetTest, ReserveDoesNotAddComponents) {
    SparseSet<Entity, Position> set;
    set.reserve(100);
    
    EXPECT_EQ(set.size(), 0);
    EXPECT_TRUE(set.empty());
}

TEST(SparseSetTest, ReserveAvoidReallocations) {
    SparseSet<Entity, Position> set;
    set.reserve(1000);

    set.emplace(0, 0.0f, 0.0f, 0.0f);
    const Position* dataPtr = set.components().data();

    for (int i = 1; i < 100; ++i) {
        set.emplace(i, i * 1.0f, i * 2.0f, i * 3.0f);
    }

    EXPECT_EQ(dataPtr, set.components().data());
}

// ============================================================================
// ITERATION TESTS
// ============================================================================

TEST(SparseSetTest, IterateOverEntities) {
    SparseSet<Entity, Position> set;
    set.emplace(0, 0.0f, 0.0f, 0.0f);
    set.emplace(5, 5.0f, 5.0f, 5.0f);
    set.emplace(10, 10.0f, 10.0f, 10.0f);
    
    std::vector<Entity> entities;
    for (auto e : set) {
        entities.push_back(e);
    }
    
    EXPECT_EQ(entities.size(), 3);
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), 0) != entities.end());
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), 5) != entities.end());
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), 10) != entities.end());
}

TEST(SparseSetTest, ComponentsAccessDirectly) {
    SparseSet<Entity, Position> set;
    set.emplace(0, 1.0f, 2.0f, 3.0f);
    set.emplace(1, 4.0f, 5.0f, 6.0f);
    
    auto& components = set.components();
    EXPECT_EQ(components.size(), 2);

    components[0].x = 100.0f;
    EXPECT_EQ(set.get(0).x, 100.0f);
}

TEST(SparseSetTest, ConstIteration) {
    SparseSet<Entity, Position> set;
    set.emplace(0, 1.0f, 2.0f, 3.0f);
    set.emplace(1, 4.0f, 5.0f, 6.0f);
    
    const auto& constSet = set;
    int count = 0;
    for (auto e : constSet) {
        count++;
        EXPECT_TRUE(constSet.contains(e));
    }
    EXPECT_EQ(count, 2);
}

// ============================================================================
// COMPLEX SEQUENCES TESTS
// ============================================================================

TEST(SparseSetTest, CreateEraseCreateSequence) {
    SparseSet<Entity, Position> set;
    
    set.emplace(0, 0.0f, 0.0f, 0.0f);
    set.emplace(1, 1.0f, 1.0f, 1.0f);
    set.emplace(2, 2.0f, 2.0f, 2.0f);
    
    set.erase(1);
    EXPECT_EQ(set.size(), 2);
    
    set.emplace(3, 3.0f, 3.0f, 3.0f);
    EXPECT_EQ(set.size(), 3);
    
    EXPECT_TRUE(set.contains(0));
    EXPECT_FALSE(set.contains(1));
    EXPECT_TRUE(set.contains(2));
    EXPECT_TRUE(set.contains(3));
}

TEST(SparseSetTest, InterleavedOperations) {
    SparseSet<Entity, Position> set;
    
    for (int i = 0; i < 10; ++i) {
        set.emplace(i, i * 1.0f, i * 2.0f, i * 3.0f);
    }

    for (int i = 0; i < 10; i += 2) {
        set.erase(i);
    }
    
    EXPECT_EQ(set.size(), 5);

    for (int i = 10; i < 15; ++i) {
        set.emplace(i, i * 1.0f, i * 2.0f, i * 3.0f);
    }
    
    EXPECT_EQ(set.size(), 10);

    for (int i = 1; i < 10; i += 2) {
        EXPECT_TRUE(set.contains(i));
    }
    for (int i = 10; i < 15; ++i) {
        EXPECT_TRUE(set.contains(i));
    }
}

TEST(SparseSetTest, MassInsertionAndDeletion) {
    SparseSet<Entity, Position> set;
    const int COUNT = 1000;

    for (int i = 0; i < COUNT; ++i) {
        set.emplace(i, i * 1.0f, i * 2.0f, i * 3.0f);
    }
    EXPECT_EQ(set.size(), COUNT);

    for (int i = 0; i < COUNT; ++i) {
        set.erase(i);
    }
    EXPECT_EQ(set.size(), 0);
    EXPECT_TRUE(set.empty());
}

// ============================================================================
// EDGE CASES TESTS
// ============================================================================

TEST(SparseSetTest, EntityZero) {
    SparseSet<Entity, Position> set;
    set.emplace(0, 1.0f, 2.0f, 3.0f);
    
    EXPECT_TRUE(set.contains(0));
    EXPECT_EQ(set.get(0).x, 1.0f);
}

TEST(SparseSetTest, VeryHighEntityId) {
    SparseSet<Entity, Position> set;
    Entity highId = 100000;
    
    set.emplace(highId, 1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(set.contains(highId));
    EXPECT_EQ(set.get(highId).x, 1.0f);
    EXPECT_EQ(set.size(), 1);
}

TEST(SparseSetTest, MultipleEraseSameEntity) {
    SparseSet<Entity, Position> set;
    set.emplace(0, 1.0f, 2.0f, 3.0f);
    
    set.erase(0);
    EXPECT_FALSE(set.contains(0));

    set.erase(0);
    EXPECT_FALSE(set.contains(0));
    EXPECT_EQ(set.size(), 0);
}

TEST(SparseSetTest, EraseAllInReverseOrder) {
    SparseSet<Entity, Position> set;
    for (int i = 0; i < 10; ++i) {
        set.emplace(i, i * 1.0f, i * 2.0f, i * 3.0f);
    }

    for (int i = 9; i >= 0; --i) {
        set.erase(i);
        EXPECT_EQ(set.size(), i);
    }
    
    EXPECT_TRUE(set.empty());
}

TEST(SparseSetTest, RandomAccessPattern) {
    SparseSet<Entity, Position> set;
    std::vector<Entity> ids = {7, 3, 15, 1, 99, 42, 8};
    
    for (auto id : ids) {
        set.emplace(id, id * 1.0f, id * 2.0f, id * 3.0f);
    }
    
    EXPECT_EQ(set.size(), ids.size());
    
    for (auto id : ids) {
        EXPECT_TRUE(set.contains(id));
        EXPECT_EQ(set.get(id).x, id * 1.0f);
    }
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

TEST(SparseSetPerformance, InsertionBenchmark) {
    SparseSet<Entity, Position> set;
    const int COUNT = 100000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < COUNT; ++i) {
        set.emplace(i, i * 1.0f, i * 2.0f, i * 3.0f);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Inserted " << COUNT << " components in " 
              << duration.count() << "ms" << std::endl;
    
    EXPECT_EQ(set.size(), COUNT);
    EXPECT_LT(duration.count(), 500);
}

TEST(SparseSetPerformance, LookupBenchmark) {
    SparseSet<Entity, Position> set;
    const int COUNT = 100000;
    
    for (int i = 0; i < COUNT; ++i) {
        set.emplace(i, i * 1.0f, i * 2.0f, i * 3.0f);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    float sum = 0.0f;
    for (int i = 0; i < COUNT; ++i) {
        sum += set.get(i).x;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Looked up " << COUNT << " components in " 
              << duration.count() << "ms (sum=" << sum << ")" << std::endl;
    
    EXPECT_LT(duration.count(), 100);
}

TEST(SparseSetPerformance, IterationBenchmark) {
    SparseSet<Entity, Position> set;
    const int COUNT = 100000;
    
    for (int i = 0; i < COUNT; ++i) {
        set.emplace(i, i * 1.0f, i * 2.0f, i * 3.0f);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    float sum = 0.0f;
    for (auto& comp : set.components()) {
        sum += comp.x;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 50);
}

TEST(SparseSetPerformance, RandomEraseBenchmark) {
    SparseSet<Entity, Position> set;
    const int COUNT = 10000;
    
    for (int i = 0; i < COUNT; ++i) {
        set.emplace(i, i * 1.0f, i * 2.0f, i * 3.0f);
    }
    
    std::vector<Entity> toErase;
    for (int i = 0; i < COUNT; i += 2) {
        toErase.push_back(i);
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(toErase.begin(), toErase.end(), gen);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (auto e : toErase) {
        set.erase(e);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(set.size(), COUNT / 2);
    EXPECT_LT(duration.count(), 100);
}

TEST(SparseSetPerformance, SparseAccessPattern) {
    SparseSet<Entity, Position> set;
    const int COUNT = 1000;

    for (int i = 0; i < COUNT; ++i) {
        set.emplace(i * 100, i * 1.0f, i * 2.0f, i * 3.0f);
    }
    
    EXPECT_EQ(set.size(), COUNT);

    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < COUNT; ++i) {
        EXPECT_TRUE(set.contains(i * 100));
        EXPECT_EQ(set.get(i * 100).x, i * 1.0f);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 50);
}

// ============================================================================
// CACHE LOCALITY TESTS
// ============================================================================

TEST(SparseSetTest, ComponentsAreContiguous) {
    SparseSet<Entity, Position> set;
    set.emplace(0, 0.0f, 0.0f, 0.0f);
    set.emplace(100, 1.0f, 1.0f, 1.0f);
    set.emplace(1000, 2.0f, 2.0f, 2.0f);
    
    auto& components = set.components();

    const Position* ptr0 = &components[0];
    const Position* ptr1 = &components[1];
    const Position* ptr2 = &components[2];

    EXPECT_EQ(ptr1 - ptr0, 1);
    EXPECT_EQ(ptr2 - ptr1, 1);
}

// ============================================================================
// DATA COHERENCES TESTS
// ============================================================================

TEST(SparseSetTest, DataIntegrityAfterMultipleOperations) {
    SparseSet<Entity, Position> set;

    set.emplace(5, 5.0f, 5.0f, 5.0f);
    set.emplace(10, 10.0f, 10.0f, 10.0f);
    set.emplace(15, 15.0f, 15.0f, 15.0f);
    
    set.erase(10);
    
    set.emplace(20, 20.0f, 20.0f, 20.0f);
    set.emplace(25, 25.0f, 25.0f, 25.0f);
    
    set.erase(5);

    EXPECT_FALSE(set.contains(5));
    EXPECT_FALSE(set.contains(10));
    EXPECT_TRUE(set.contains(15));
    EXPECT_TRUE(set.contains(20));
    EXPECT_TRUE(set.contains(25));
    
    EXPECT_EQ(set.get(15).x, 15.0f);
    EXPECT_EQ(set.get(20).x, 20.0f);
    EXPECT_EQ(set.get(25).x, 25.0f);
    
    EXPECT_EQ(set.size(), 3);
}

TEST(SparseSetTest, AllEntitiesAccessibleAfterErase) {
    SparseSet<Entity, Position> set;
    
    for (int i = 0; i < 20; ++i) {
        set.emplace(i, i * 1.0f, i * 2.0f, i * 3.0f);
    }

    set.erase(5);
    set.erase(10);
    set.erase(15);

    for (int i = 0; i < 20; ++i) {
        if (i == 5 || i == 10 || i == 15) {
            EXPECT_FALSE(set.contains(i));
        } else {
            EXPECT_TRUE(set.contains(i));
            EXPECT_EQ(set.get(i).x, i * 1.0f);
        }
    }
}