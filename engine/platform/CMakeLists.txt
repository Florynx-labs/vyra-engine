add_library(vyra_platform STATIC
    include/vyra/platform/event.hpp
    include/vyra/platform/key_codes.hpp
    include/vyra/platform/mouse_codes.hpp
    include/vyra/platform/window.hpp
    include/vyra/platform/input.hpp
    src/sdl3_window.hpp
    src/sdl3_window.cpp
    src/sdl3_input.cpp
)

add_library(vyra::platform ALIAS vyra_platform)

target_include_directories(vyra_platform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(vyra_platform
    PUBLIC
        vyra::core
        SDL3::SDL3-static
    PRIVATE
        imgui
)

vyra_apply_compiler_flags(vyra_platform)
