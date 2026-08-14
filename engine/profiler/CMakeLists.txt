# VYRA Engine Profiler System

add_library(vyra_profiler STATIC
    # Headers
    include/vyra/profiler/profiler.hpp
    
    # Sources
    src/profiler.cpp
)

target_include_directories(vyra_profiler
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(vyra_profiler
    PUBLIC
        vyra_core
)

target_compile_features(vyra_profiler PUBLIC cxx_std_23)
