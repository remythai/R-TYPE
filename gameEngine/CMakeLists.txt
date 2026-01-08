cmake_minimum_required(VERSION 3.15)
project(GameEngine_ECS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ECS_BUILD_COMPONENTS "Build all component libraries" ON)
option(ECS_BUILD_SYSTEMS "Build all system libraries" ON)
option(ECS_VERBOSE_DISCOVERY "Show verbose component/system discovery" ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message(STATUS "Binary dir: ${CMAKE_BINARY_DIR}")
message(STATUS "Library output: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

add_library(ecs_core INTERFACE)

target_include_directories(ecs_core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/ecs
)

if(ECS_BUILD_COMPONENTS)
    message(STATUS "\n========================================")
    message(STATUS "Building ECS Components")
    message(STATUS "========================================")
    add_subdirectory(components)
endif()

if(ECS_BUILD_SYSTEMS)
    message(STATUS "\n========================================")
    message(STATUS "Building ECS Systems")
    message(STATUS "========================================")
    add_subdirectory(systems)
endif()

message(STATUS "\n========================================")
message(STATUS "ECS Build Configuration Summary")
message(STATUS "========================================")
message(STATUS "Components discovered: ${ECS_COMPONENTS}")
message(STATUS "Systems discovered: ${ECS_SYSTEMS}")
message(STATUS "Build directory: ${CMAKE_BINARY_DIR}")
message(STATUS "========================================\n")

install(DIRECTORY ecs/
    DESTINATION include/ecs
    FILES_MATCHING PATTERN "*.hpp"
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ecs_config.h.in")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ecs_config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ecs_config.h"
    )
    
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ecs_config.h"
        DESTINATION include/ecs
    )
else()
    message(STATUS "Note: ecs_config.h.in not found, skipping configuration file")
endif()

# =======================
# Google Benchmark (FetchContent)
# =======================
include(FetchContent)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.4       # version stable au lieu de main
)

# Optionnel: désactiver les tests de Google Benchmark
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable benchmark tests" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark testing" FORCE)

FetchContent_MakeAvailable(benchmark)

# Exécutable de benchmark ECS
add_executable(ecs_bench
    tests/ecs_bench.cpp  # à créer
)

target_link_libraries(ecs_bench
    PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main
        ecs_core           # + ajoute ici tes libs de systems/components si elles existent
)

target_compile_definitions(ecs_bench
    PRIVATE
        ECS_DISABLE_LOGS
)
