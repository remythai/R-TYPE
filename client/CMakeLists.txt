cmake_minimum_required(VERSION 3.16)
project(r-type_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
)

set(SFML_BUILD_AUDIO TRUE)
set(SFML_BUILD_GRAPHICS TRUE)
set(SFML_BUILD_WINDOW TRUE)
set(SFML_BUILD_SYSTEM TRUE)
set(SFML_BUILD_NETWORK FALSE)

FetchContent_MakeAvailable(SFML)

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-30-2
)

FetchContent_MakeAvailable(asio)

set(SOURCES
    src/main.cpp
    src/core/Core.cpp
    src/graphics/Window.cpp
    src/graphics/AnimatedSprite.cpp
    src/graphics/ResourceManager.cpp
    src/graphics/EntityManager.cpp
    src/graphics/ParallaxSystem.cpp
    src/network/NetworkClient.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/include
    ${asio_SOURCE_DIR}/asio/include
)

target_compile_definitions(${PROJECT_NAME} PRIVATE ASIO_STANDALONE)