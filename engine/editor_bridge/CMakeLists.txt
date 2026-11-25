add_library(vyra_editor_bridge SHARED
    include/vyra/editor_bridge/editor_bridge.h
    src/editor_bridge.cpp
)

add_library(vyra::editor_bridge ALIAS vyra_editor_bridge)

# The bridge is shared, while its engine dependencies remain static.
# Mark those dependencies PIC so they can safely be linked into this library.
set_target_properties(vyra_core vyra_ecs vyra_scene PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

set_target_properties(vyra_editor_bridge PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

target_include_directories(vyra_editor_bridge
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(vyra_editor_bridge PUBLIC vyra::scene)
vyra_apply_compiler_flags(vyra_editor_bridge)
