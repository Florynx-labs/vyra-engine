cmake_minimum_required(VERSION 3.28)

project(VYRA
    VERSION 0.1.0
    DESCRIPTION "Modern Open-Source Multiplatform C++23 Game Engine"
    HOMEPAGE_URL "https://github.com/florynx-labs/vyra-engine"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VYRA_BUILD_TESTS "Build VYRA unit test suite" ON)
option(VYRA_BUILD_SAMPLES "Build VYRA sample applications" ON)

include(cmake/VyraCompilerFlags.cmake)
include(cmake/VyraDependencies.cmake)

# Subdirectories
add_subdirectory(engine/core)
add_subdirectory(engine/platform)
add_subdirectory(engine/rhi)
add_subdirectory(engine/ecs)
add_subdirectory(engine/scene)

if(VYRA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
