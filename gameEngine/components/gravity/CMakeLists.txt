
cmake_minimum_required(VERSION 3.15)

set(COMPONENT_NAME gravity)
project(component_${COMPONENT_NAME} VERSION 1.0.0)

set(PLUGIN_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT_NAME}.cpp")

file(WRITE ${PLUGIN_SOURCE}
"#include \"../src/Gravity.hpp\"

extern \"C\" {
    IComponent* createComponent() {
        return new GameEngine::Gravity();
    }
    
    void destroyComponent(IComponent* component) {
        delete component;
    }
    
    const char* getComponentName() {
        return GameEngine::Gravity::Name;
    }
    
    const char* getComponentVersion() {
        return GameEngine::Gravity::Version;
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

install(TARGETS ${COMPONENT_NAME}
    LIBRARY DESTINATION lib/components
    ARCHIVE DESTINATION lib/components
    RUNTIME DESTINATION bin/components
)

install(FILES src/Gravity.hpp
    DESTINATION include/components
)


install(FILES ${COMPONENT_INFO_FILE}
    DESTINATION share/components
)
