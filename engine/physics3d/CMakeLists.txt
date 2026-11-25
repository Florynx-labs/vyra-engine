add_library(vyra_physics3d STATIC
    include/vyra/physics3d/physics3d.hpp
    src/physics3d.cpp
)

add_library(vyra::physics3d ALIAS vyra_physics3d)

target_include_directories(vyra_physics3d
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(vyra_physics3d PUBLIC vyra::core glm::glm)
vyra_apply_compiler_flags(vyra_physics3d)
