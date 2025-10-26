
cmake_minimum_required(VERSION 3.15)

set(COMPONENT_NAME velocity)
project(component_${COMPONENT_NAME} VERSION 1.0.0)

set(PLUGIN_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT_NAME}.cpp")

file(WRITE ${PLUGIN_SOURCE}
"#include \"../src/Velocity.hpp\"

extern \"C\" {
    IComponent* createVelocity() {
        return new GameEngine::Velocity();
    }
    
    void destroyVelocity(IComponent* component) {
        delete component;
    }
    
    const char* getComponentName() {
        return GameEngine::Velocity::Name;
    }
    
    const char* getComponentVersion() {
        return GameEngine::Velocity::Version;
    }
}
")

add_library(${COMPONENT_NAME} SHARED
    ${PLUGIN_SOURCE}
)

set_target_properties(${COMPONENT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "${COMPONENT_NAME}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/components"
)

target_include_directories(${COMPONENT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/ecs
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(${COMPONENT_NAME} PRIVATE
    -fvisibility=default
)

# Installation
install(TARGETS ${COMPONENT_NAME}
    LIBRARY DESTINATION lib/components
    ARCHIVE DESTINATION lib/components
    RUNTIME DESTINATION bin/components
)

install(FILES src/Velocity.hpp
    DESTINATION include/components
)


install(FILES ${COMPONENT_INFO_FILE}
    DESTINATION share/components
)
