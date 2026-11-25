add_library(vyra_animation INTERFACE)
add_library(vyra::animation ALIAS vyra_animation)
target_include_directories(vyra_animation INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(vyra_animation INTERFACE vyra::core glm::glm)
target_compile_features(vyra_animation INTERFACE cxx_std_23)
