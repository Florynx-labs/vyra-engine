# Header-only gameplay runtime primitives. Platform-specific backends remain in their owning modules.
add_library(vyra_runtime INTERFACE)
add_library(vyra::runtime ALIAS vyra_runtime)

target_include_directories(vyra_runtime INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(vyra_runtime INTERFACE vyra::core)
target_compile_features(vyra_runtime INTERFACE cxx_std_23)
