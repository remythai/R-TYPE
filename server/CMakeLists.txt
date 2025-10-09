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

set(SOURCES
    src/main.cpp
    src/network/NetworkServer.cpp
    src/network/handleClient.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Add include path for ASIO (header-only)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${asio_SOURCE_DIR}/asio/include
)
