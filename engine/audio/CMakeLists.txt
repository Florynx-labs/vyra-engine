add_library(vyra_audio INTERFACE)
add_library(vyra::audio ALIAS vyra_audio)
target_include_directories(vyra_audio INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(vyra_audio INTERFACE vyra::core glm::glm)
target_compile_features(vyra_audio INTERFACE cxx_std_23)
