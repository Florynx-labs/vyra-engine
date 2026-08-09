add_library(vyra_rhi STATIC
    include/vyra/rhi/rhi.hpp
    include/vyra/rhi/rhi_resources.hpp
    src/vulkan/vulkan_context.hpp
    src/vulkan/vulkan_context.cpp
    src/vulkan/vulkan_device.hpp
    src/vulkan/vulkan_device.cpp
    src/vulkan/vulkan_swapchain.hpp
    src/vulkan/vulkan_swapchain.cpp
    src/vulkan/vulkan_resources.hpp
    src/vulkan/vulkan_resources.cpp
    src/vulkan/vulkan_sync.hpp
    src/vulkan/vulkan_sync.cpp
)

add_library(vyra::rhi ALIAS vyra_rhi)

target_include_directories(vyra_rhi
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(vyra_rhi
    PUBLIC
        vyra::core
    PRIVATE
        vyra::platform
        volk::volk
        Vulkan::Headers
)

# Volk needs VK_NO_PROTOTYPES defined
target_compile_definitions(vyra_rhi PRIVATE VK_NO_PROTOTYPES)

vyra_apply_compiler_flags(vyra_rhi)
