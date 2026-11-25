add_library(vyra_scripting INTERFACE)
add_library(vyra::scripting ALIAS vyra_scripting)
target_include_directories(vyra_scripting INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(vyra_scripting INTERFACE vyra::core)
target_compile_features(vyra_scripting INTERFACE cxx_std_23)
