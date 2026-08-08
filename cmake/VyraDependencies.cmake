include_guard(GLOBAL)
include(FetchContent)

# 1. spdlog (Asynchronous Logging)
message(STATUS "[VYRA Dependencies] Configuring spdlog...")
FetchContent_Declare(
    spdlog
    URL https://github.com/gabime/spdlog/archive/refs/tags/v1.13.0.zip
)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(spdlog)

# 2. GLM (OpenGL Mathematics)
message(STATUS "[VYRA Dependencies] Configuring GLM...")
FetchContent_Declare(
    glm
    URL https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

# 3. nlohmann_json (JSON Serialization)
message(STATUS "[VYRA Dependencies] Configuring nlohmann_json...")
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.zip
)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(nlohmann_json)

# 4. Catch2 (Unit Testing Framework)
if(VYRA_BUILD_TESTS)
    message(STATUS "[VYRA Dependencies] Configuring Catch2 for testing...")
    FetchContent_Declare(
        Catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.5.2.zip
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)
endif()
