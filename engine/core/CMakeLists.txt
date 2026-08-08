add_library(vyra_core STATIC
    include/vyra/core/base.hpp
    include/vyra/core/log.hpp
    include/vyra/core/assert.hpp
    include/vyra/core/uuid.hpp
    include/vyra/core/time.hpp
    src/log.cpp
    src/uuid.cpp
    src/time.cpp
)

add_library(vyra::core ALIAS vyra_core)

target_include_directories(vyra_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(vyra_core
    PUBLIC
        spdlog::spdlog
        glm::glm
)

vyra_apply_compiler_flags(vyra_core)
