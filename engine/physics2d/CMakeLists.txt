add_library(vyra_physics2d STATIC
    include/vyra/physics2d/physics2d.hpp
    src/physics2d.cpp
    src/world.cpp
)

add_library(vyra::physics2d ALIAS vyra_physics2d)

target_include_directories(vyra_physics2d
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(vyra_physics2d PUBLIC vyra::core glm::glm)
vyra_apply_compiler_flags(vyra_physics2d)
