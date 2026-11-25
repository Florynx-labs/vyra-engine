add_library(vyra_editor_bridge STATIC
    include/vyra/editor_bridge/editor_bridge.h
    src/editor_bridge.cpp
)

add_library(vyra::editor_bridge ALIAS vyra_editor_bridge)

target_include_directories(vyra_editor_bridge
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(vyra_editor_bridge
    PUBLIC
        vyra::scene
)

vyra_apply_compiler_flags(vyra_editor_bridge)
