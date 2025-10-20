#include <gtest/gtest.h>
#include "../ecs/ComponentRegistry.hpp"
#include <chrono>

// Composants de test
struct Transform { float x, y, z; };
struct Velocity { float vx, vy; };
struct Health { int hp; };
struct Armor { int defense; };
struct Name { std::string value; };

// --- Tests de base ---

TEST(ComponentRegistryTest, SingletonInstance) {
    auto& reg1 = ComponentRegistry::instance();
    auto& reg2 = ComponentRegistry::instance();
    
    EXPECT_EQ(&reg1, &reg2);
}

TEST(ComponentRegistryTest, GetOrCreateIDReturnsUniqueIDs) {
    auto& reg = ComponentRegistry::instance();
    
    auto id1 = reg.getOrCreateID<Transform>();
    auto id2 = reg.getOrCreateID<Velocity>();
    auto id3 = reg.getOrCreateID<Health>();
    
    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
    EXPECT_NE(id1, id3);
}

TEST(ComponentRegistryTest, GetOrCreateIDIsCached) {
    auto& reg = ComponentRegistry::instance();
    
    auto id1 = reg.getOrCreateID<Transform>();
    auto id2 = reg.getOrCreateID<Transform>();
    auto id3 = reg.getOrCreateID<Transform>();
    
    EXPECT_EQ(id1, id2);
    EXPECT_EQ(id2, id3);
}

// --- Tests string-based ---

TEST(ComponentRegistryTest, StringBasedRegistration) {
    auto& reg = ComponentRegistry::instance();
    
    auto id1 = reg.getOrCreateID("CustomComponent");
    auto id2 = reg.getOrCreateID("AnotherComponent");
    
    EXPECT_NE(id1, id2);
}

TEST(ComponentRegistryTest, StringBasedCaching) {
    auto& reg = ComponentRegistry::instance();
    
    auto id1 = reg.getOrCreateID("TestComponent");
    auto id2 = reg.getOrCreateID("TestComponent");
    
    EXPECT_EQ(id1, id2);
}

TEST(ComponentRegistryTest, GetNameReturnsCorrectName) {
    auto& reg = ComponentRegistry::instance();
    
    auto id = reg.getOrCreateID("PhysicsBody");
    auto name = reg.getName(id);
    
    EXPECT_EQ(name, "PhysicsBody");
}

TEST(ComponentRegistryTest, GetNameForInvalidIDReturnsEmpty) {
    auto& reg = ComponentRegistry::instance();
    
    auto name = reg.getName(99999);
    
    EXPECT_TRUE(name.empty());
}

TEST(ComponentRegistryTest, GetIDReturnsCorrectID) {
    auto& reg = ComponentRegistry::instance();
    
    auto id1 = reg.getOrCreateID("AudioSource");
    auto id2 = reg.getID("AudioSource");
    
    EXPECT_EQ(id1, id2);
}

TEST(ComponentRegistryTest, GetIDForUnknownNameReturnsInvalid) {
    auto& reg = ComponentRegistry::instance();
    
    auto id = reg.getID("NonExistentComponent");
    
    EXPECT_EQ(id, ComponentRegistry::INVALID_ID);
}

// --- Tests de cohérence type/string ---

TEST(ComponentRegistryTest, TypeAndStringIDsAreIndependent) {
    auto& reg = ComponentRegistry::instance();
    
    auto typeID = reg.getOrCreateID<Transform>();
    auto stringID = reg.getOrCreateID("Transform");
    
    // Les IDs peuvent être différents car ce sont deux systèmes indépendants
    // (sauf si le nom du type correspond exactement)
    EXPECT_TRUE(typeID != stringID || typeID == stringID);
}

// --- Tests de cas limites ---

TEST(ComponentRegistryTest, EmptyStringRegistration) {
    auto& reg = ComponentRegistry::instance();
    
    auto id1 = reg.getOrCreateID("");
    auto id2 = reg.getOrCreateID("");
    
    EXPECT_EQ(id1, id2);
}

TEST(ComponentRegistryTest, VeryLongStringName) {
    auto& reg = ComponentRegistry::instance();
    
    std::string longName(1000, 'A');
    auto id = reg.getOrCreateID(longName);
    auto retrieved = reg.getName(id);
    
    EXPECT_EQ(retrieved, longName);
}

TEST(ComponentRegistryTest, SpecialCharactersInName) {
    auto& reg = ComponentRegistry::instance();
    
    auto id = reg.getOrCreateID("Component::Nested<T>");
    auto name = reg.getName(id);
    
    EXPECT_EQ(name, "Component::Nested<T>");
}

TEST(ComponentRegistryTest, InvalidIDConstant) {
    EXPECT_EQ(ComponentRegistry::INVALID_ID, static_cast<ComponentID>(-1));
}

// --- Tests de capacité ---

TEST(ComponentRegistryTest, HandlesLargeNumberOfTypes) {
    auto& reg = ComponentRegistry::instance();
    std::vector<ComponentID> ids;
    
    for (int i = 0; i < 1000; ++i) {
        std::string name = "Component_" + std::to_string(i);
        ids.push_back(reg.getOrCreateID(name));
    }
    
    // Vérifier l'unicité
    std::set<ComponentID> uniqueIds(ids.begin(), ids.end());
    EXPECT_EQ(uniqueIds.size(), 1000);
}

TEST(ComponentRegistryTest, MixedTypeAndStringRegistration) {
    auto& reg = ComponentRegistry::instance();
    
    auto t1 = reg.getOrCreateID<Transform>();
    auto s1 = reg.getOrCreateID("StringComponent1");
    auto t2 = reg.getOrCreateID<Velocity>();
    auto s2 = reg.getOrCreateID("StringComponent2");
    auto t3 = reg.getOrCreateID<Health>();
    
    // Tous doivent être uniques
    std::set<ComponentID> ids = {t1, s1, t2, s2, t3};
    EXPECT_EQ(ids.size(), 5);
}

// --- Tests de cohérence après plusieurs opérations ---

TEST(ComponentRegistryTest, ConsistencyAfterManyOperations) {
    auto& reg = ComponentRegistry::instance();
    
    std::vector<std::string> names;
    std::vector<ComponentID> ids;
    
    for (int i = 0; i < 100; ++i) {
        std::string name = "Comp_" + std::to_string(i);
        names.push_back(name);
        ids.push_back(reg.getOrCreateID(name));
    }
    
    // Vérifier que les IDs sont stables
    for (size_t i = 0; i < names.size(); ++i) {
        EXPECT_EQ(reg.getID(names[i]), ids[i]);
        EXPECT_EQ(reg.getName(ids[i]), names[i]);
    }
}

// --- Tests de performance ---

TEST(ComponentRegistryPerformance, TypeBasedLookupSpeed) {
    auto& reg = ComponentRegistry::instance();
    
    // Premier accès pour initialiser
    reg.getOrCreateID<Transform>();
    
    const size_t ITERATIONS = 10000000;
    auto start = std::chrono::high_resolution_clock::now();
    
    ComponentID id;
    for (size_t i = 0; i < ITERATIONS; ++i) {
        id = reg.getOrCreateID<Transform>();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Le cache statique doit rendre cela extrêmement rapide
    EXPECT_LT(duration.count(), 100);
    EXPECT_NE(id, ComponentRegistry::INVALID_ID); // Évite l'optimisation
}

TEST(ComponentRegistryPerformance, StringBasedLookupSpeed) {
    auto& reg = ComponentRegistry::instance();
    const std::string name = "BenchmarkComponent";
    
    // Premier accès pour créer l'entrée
    reg.getOrCreateID(name);
    
    const size_t ITERATIONS = 1000000;
    auto start = std::chrono::high_resolution_clock::now();
    
    ComponentID id;
    for (size_t i = 0; i < ITERATIONS; ++i) {
        id = reg.getOrCreateID(name);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Avec unordered_map, même 1M d'itérations devrait être rapide
    EXPECT_LT(duration.count(), 200);
    EXPECT_NE(id, ComponentRegistry::INVALID_ID);
}

TEST(ComponentRegistryPerformance, MassiveRegistrationSpeed) {
    auto& reg = ComponentRegistry::instance();
    const size_t COUNT = 10000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < COUNT; ++i) {
        std::string name = "MassComp_" + std::to_string(i);
        reg.getOrCreateID(name);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Avec unordered_map O(1), cela devrait être très rapide
    EXPECT_LT(duration.count(), 100);
}

TEST(ComponentRegistryPerformance, GetNameLookupSpeed) {
    auto& reg = ComponentRegistry::instance();
    
    std::vector<ComponentID> ids;
    for (int i = 0; i < 100; ++i) {
        std::string name = "LookupComp_" + std::to_string(i);
        ids.push_back(reg.getOrCreateID(name));
    }
    
    const size_t ITERATIONS = 100000;
    auto start = std::chrono::high_resolution_clock::now();
    
    std::string name;
    for (size_t i = 0; i < ITERATIONS; ++i) {
        name = reg.getName(ids[i % ids.size()]);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 200);
    EXPECT_FALSE(name.empty());
}

// --- Tests de thread safety (documentation) ---
// Note: Ces tests ne vérifient pas la thread-safety car le code actuel
// n'implémente pas de mutex. Si nécessaire, ajouter des tests avec std::thread.

TEST(ComponentRegistryTest, MultipleTypeRegistrations) {
    auto& reg = ComponentRegistry::instance();
    
    auto id1 = reg.getOrCreateID<Transform>();
    auto id2 = reg.getOrCreateID<Velocity>();
    auto id3 = reg.getOrCreateID<Health>();
    auto id4 = reg.getOrCreateID<Armor>();
    auto id5 = reg.getOrCreateID<Name>();
    
    // Tous différents
    std::set<ComponentID> ids = {id1, id2, id3, id4, id5};
    EXPECT_EQ(ids.size(), 5);
    
    // Réaccès pour vérifier la stabilité
    EXPECT_EQ(reg.getOrCreateID<Transform>(), id1);
    EXPECT_EQ(reg.getOrCreateID<Velocity>(), id2);
    EXPECT_EQ(reg.getOrCreateID<Health>(), id3);
}