add_library(vyra_renderer STATIC
    include/vyra/renderer/mesh.hpp
    include/vyra/renderer/mesh_renderer.hpp
    include/vyra/renderer/sprite_renderer.hpp
    src/mesh.cpp
    src/mesh_renderer.cpp
    src/sprite_renderer.cpp
)

add_library(vyra::renderer ALIAS vyra_renderer)

target_include_directories(vyra_renderer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(vyra_renderer
    PUBLIC
        vyra::core
        vyra::rhi
        glm::glm
    PRIVATE
        volk::volk
        Vulkan::Headers
)

target_compile_definitions(vyra_renderer PRIVATE VK_NO_PROTOTYPES)

vyra_apply_compiler_flags(vyra_renderer)
