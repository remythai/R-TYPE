cmake_minimum_required(VERSION 3.16)
project(EntityManagerTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(runTests
    entityManager_tests.cpp
    sparseSet_tests.cpp
    componentRegistry_tests.cpp
    registry_tests.cpp
)

target_link_libraries(runTests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(runTests)
