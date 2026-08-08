add_executable(vyra_tests
    unit/test_core.cpp
    unit/test_platform.cpp
    unit/test_rhi.cpp
)

target_link_libraries(vyra_tests
    PRIVATE
        vyra::core
        vyra::platform
        vyra::rhi
        Catch2::Catch2WithMain
)

vyra_apply_compiler_flags(vyra_tests)

catch_discover_tests(vyra_tests)
