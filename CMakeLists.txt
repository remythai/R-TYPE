cmake_minimum_required(VERSION 3.16)
project(r-type)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_TESTS "Enable unit tests" OFF)

add_subdirectory(client)
add_subdirectory(server)

add_custom_target(all_projects
    DEPENDS r-type_client r-type_server
)

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/r-type_client${CMAKE_EXECUTABLE_SUFFIX}
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/r-type_server${CMAKE_EXECUTABLE_SUFFIX}
    COMMENT "Cleaning all executables"
)