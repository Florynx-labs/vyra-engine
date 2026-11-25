add_executable(vyra_tests
    unit/test_core.cpp
    unit/test_platform.cpp
    unit/test_rhi.cpp
    unit/test_ecs.cpp
    unit/test_scene.cpp
    unit/test_editor_bridge.cpp
    unit/test_renderer.cpp
    unit/test_physics2d.cpp
    unit/test_physics_world.cpp
    unit/test_physics3d.cpp
    unit/test_asset.cpp
    unit/test_profiler.cpp
    unit/test_jobs.cpp
    unit/test_memory_pool.cpp
    unit/test_runtime.cpp
)

target_link_libraries(vyra_tests
    PRIVATE
        vyra_core
        vyra_platform
        vyra_rhi
        vyra_renderer
        vyra_physics2d
        vyra_physics3d
        vyra_ecs
        vyra_scene
        vyra_editor_bridge
        vyra_asset
        vyra_profiler
        vyra_jobs
        Catch2::Catch2WithMain
)

vyra_apply_compiler_flags(vyra_tests)

catch_discover_tests(vyra_tests)

# GPU integration tests require a Vulkan-capable runtime and are optional in headless CI.
if(VYRA_BUILD_GPU_TESTS)
    add_executable(vyra_gpu_tests
        gpu/test_vulkan_smoke.cpp
        gpu/test_vulkan_integration.cpp
    )

    target_link_libraries(vyra_gpu_tests
        PRIVATE
            vyra_core
            vyra_rhi
            vyra_asset
            Catch2::Catch2WithMain
    )

    vyra_apply_compiler_flags(vyra_gpu_tests)
    catch_discover_tests(vyra_gpu_tests)
endif()
