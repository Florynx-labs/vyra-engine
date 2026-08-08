#pragma once

#include "vyra/core/base.hpp"
#include "vyra/core/log.hpp"
#include <filesystem>

#if defined(VYRA_DEBUG)
    #if defined(VYRA_PLATFORM_WINDOWS)
        #define VYRA_DEBUGBREAK() __debugbreak()
    #elif defined(VYRA_PLATFORM_LINUX) || defined(VYRA_PLATFORM_MACOS)
        #include <signal.h>
        #define VYRA_DEBUGBREAK() raise(SIGTRAP)
    #else
        #define VYRA_DEBUGBREAK()
    #endif
    #define VYRA_ENABLE_ASSERTS
#else
    #define VYRA_DEBUGBREAK()
#endif

#if defined(VYRA_ENABLE_ASSERTS)
    #define VYRA_ASSERT_IMPL(type, check, msg, ...) \
        do { \
            if (!(check)) { \
                VYRA_LOG_##type##_ERROR("Assertion Failed: {0} at {1}:{2}", msg, ::std::filesystem::path(__FILE__).filename().string(), __LINE__); \
                VYRA_DEBUGBREAK(); \
            } \
        } while (false)

    #define VYRA_ASSERT(check, ...) VYRA_ASSERT_IMPL(CORE, check, __VA_ARGS__)
    #define VYRA_CORE_ASSERT(check, ...) VYRA_ASSERT_IMPL(CORE, check, __VA_ARGS__)
#else
    #define VYRA_ASSERT(check, ...)
    #define VYRA_CORE_ASSERT(check, ...)
#endif
