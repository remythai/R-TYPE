cmake_minimum_required(VERSION 3.16)
project(EntityManagerTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Importer GoogleTest via FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

# Activer le framework de tests CMake
enable_testing()

# Ajouter l’exécutable de tests
add_executable(runTests
    entityManager_tests.cpp
    sparseSet_tests.cpp
)

# Lier GoogleTest
target_link_libraries(runTests PRIVATE GTest::gtest_main)

# Déclarer les tests automatiquement pour CTest
include(GoogleTest)
gtest_discover_tests(runTests)
