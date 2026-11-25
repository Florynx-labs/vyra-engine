add_library(vyra_input INTERFACE)
add_library(vyra::input ALIAS vyra_input)
target_include_directories(vyra_input INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(vyra_input INTERFACE vyra::core)
target_compile_features(vyra_input INTERFACE cxx_std_23)
