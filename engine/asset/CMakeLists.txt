# VYRA Engine Asset System

# Create library
add_library(vyra_asset STATIC
    # Headers
    include/vyra/asset/asset_handle.hpp
    include/vyra/asset/asset_id.hpp
    include/vyra/asset/asset_metadata.hpp
    include/vyra/asset/asset_registry.hpp
    
    # Sources
    src/asset_registry.cpp
)

# Include directories
target_include_directories(vyra_asset
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(vyra_asset
    PUBLIC
        vyra_core
)

# Compiler flags
target_compile_features(vyra_asset PUBLIC cxx_std_23)
