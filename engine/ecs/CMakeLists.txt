add_library(vyra_ecs STATIC
    include/vyra/ecs/ecs.hpp
    src/ecs.cpp
)

add_library(vyra::ecs ALIAS vyra_ecs)

target_include_directories(vyra_ecs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(vyra_ecs
    PUBLIC
        vyra::core
        EnTT::EnTT
        glm::glm
)

vyra_apply_compiler_flags(vyra_ecs)
