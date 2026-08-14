# VYRA Engine Job System

add_library(vyra_jobs STATIC
    # Headers
    include/vyra/jobs/job_system.hpp
    
    # Sources
    src/job_system.cpp
)

target_include_directories(vyra_jobs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(vyra_jobs
    PUBLIC
        vyra_core
)

target_compile_features(vyra_jobs PUBLIC cxx_std_23)
