add_library(vyra_scene STATIC
    include/vyra/scene/scene.hpp
    include/vyra/scene/scene_camera.hpp
    include/vyra/scene/components.hpp
    include/vyra/scene/scene_serializer.hpp
    src/scene.cpp
    src/scene_serializer.cpp
)

add_library(vyra::scene ALIAS vyra_scene)

target_include_directories(vyra_scene PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(vyra_scene PUBLIC
    vyra::core
    vyra::ecs
    glm::glm
    nlohmann_json::nlohmann_json
)

vyra_apply_compiler_flags(vyra_scene)

