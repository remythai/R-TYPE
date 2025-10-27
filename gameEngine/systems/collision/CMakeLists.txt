
cmake_minimum_required(VERSION 3.15)

set(SYSTEM_NAME collision)
project(system_${SYSTEM_NAME} VERSION 1.0.0)

add_library(${SYSTEM_NAME} SHARED
    src/Collision.cpp
)

set_target_properties(${SYSTEM_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "${SYSTEM_NAME}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/systems"
)

target_compile_options(${SYSTEM_NAME} PRIVATE
    -fvisibility=default
)

install(TARGETS ${SYSTEM_NAME}
    LIBRARY DESTINATION lib/systems
    ARCHIVE DESTINATION lib/systems
    RUNTIME DESTINATION bin/systems
)

install(FILES src/Collision.hpp
    DESTINATION include/systems
)

install(FILES ${SYSTEM_INFO_FILE}
    DESTINATION share/systems
)
