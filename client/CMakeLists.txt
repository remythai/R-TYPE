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

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.5
)

FetchContent_MakeAvailable(imgui)

if(EXISTS "${CMAKE_SOURCE_DIR}/external/imgui-sfml/CMakeLists.txt")
    message(STATUS "Using ImGui-SFML from submodule")
    set(IMGUI_DIR ${imgui_SOURCE_DIR})
    set(IMGUI_SFML_FIND_SFML OFF)
    set(IMGUI_SFML_IMGUI_DEMO ON)
    add_subdirectory(external/imgui-sfml imgui-sfml)
else()
    message(STATUS "Using ImGui-SFML from FetchContent")
    FetchContent_Declare(
        imgui-sfml
        GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
        GIT_TAG master
    )
    set(IMGUI_DIR ${imgui_SOURCE_DIR})
    set(IMGUI_SFML_FIND_SFML OFF)
    set(IMGUI_SFML_IMGUI_DEMO ON)
    FetchContent_MakeAvailable(imgui-sfml)
endif()

set(SOURCES
    src/main.cpp

    src/core/Core.cpp

    src/graphics/Window.cpp
    src/graphics/ResourceManager.cpp
    src/graphics/EntityManager.cpp
    src/graphics/ParallaxSystem.cpp

    src/keybind/KeybindManager.cpp
    src/keybind/KeybindMenu.cpp

    src/network/NetworkClient.cpp

    src/mapEditor/MapEditor.cpp

    src/colorBlindFilter/ColorBlindFilter.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    ImGui-SFML::ImGui-SFML
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/include
    ${asio_SOURCE_DIR}/asio/include
)

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