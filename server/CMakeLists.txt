cmake_minimum_required(VERSION 3.16)
project(r-type_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(SOURCES
    src/main.cpp
    src/network/NetworkServer.cpp
    src/network/handleClient.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})