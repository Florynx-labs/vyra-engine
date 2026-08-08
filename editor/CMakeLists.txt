add_executable(vyra_editor
    include/vyra/editor/editor_layer.hpp
    include/vyra/editor/editor_theme.hpp
    include/vyra/editor/panels/viewport_panel.hpp
    include/vyra/editor/panels/hierarchy_panel.hpp
    include/vyra/editor/panels/inspector_panel.hpp
    include/vyra/editor/panels/console_panel.hpp
    src/editor_layer.cpp
    src/editor_theme.cpp
    src/panels/viewport_panel.cpp
    src/panels/hierarchy_panel.cpp
    src/panels/inspector_panel.cpp
    src/panels/console_panel.cpp
    src/main.cpp
)

target_include_directories(vyra_editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(vyra_editor PRIVATE
    vyra::core
    vyra::platform
    vyra::rhi
    vyra::ecs
    vyra::scene
    imgui
    SDL3::SDL3
    glm::glm
)

vyra_apply_compiler_flags(vyra_editor)
