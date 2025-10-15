cmake_minimum_required(VERSION 3.16)
project(r-type_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

include(FetchContent)

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-30-2
)

FetchContent_MakeAvailable(asio)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(SOURCES
    src/main.cpp
    src/network/NetworkServer.cpp
    src/network/handleClient.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${asio_SOURCE_DIR}/asio/include
)

file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)

if(TEST_SOURCES)
    add_executable(r-type_tests
        ${TEST_SOURCES}
        src/network/NetworkServer.cpp
        src/network/handleClient.cpp
    )

    target_include_directories(r-type_tests PRIVATE
        ${asio_SOURCE_DIR}/asio/include
        src/
    )

    target_link_libraries(r-type_tests
        PRIVATE
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(r-type_tests)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ASIO_STANDALONE
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0601>
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32 wsock32)
endif()

if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} Threads::Threads)
endif()