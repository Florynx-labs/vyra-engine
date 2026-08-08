#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <utility>
#include <string>
#include <string_view>

// Platform Detection
#if defined(_WIN32) || defined(_WIN64)
    #define VYRA_PLATFORM_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC == 1
        #define VYRA_PLATFORM_MACOS
    #else
        #error "Unsupported Apple Platform"
    #endif
#elif defined(__linux__)
    #define VYRA_PLATFORM_LINUX
#else
    #error "Unsupported Target Platform"
#endif

// DLL Export / Import Macros
#if defined(VYRA_PLATFORM_WINDOWS)
    #if defined(VYRA_DYNAMIC_LINK)
        #if defined(VYRA_BUILD_DLL)
            #define VYRA_API __declspec(dllexport)
        #else
            #define VYRA_API __declspec(dllimport)
        #endif
    #else
        #define VYRA_API
    #endif
#else
    #define VYRA_API __attribute__((visibility("default")))
#endif

#define VYRA_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace vyra {

    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T, typename ... Args>
    constexpr Ref<T> CreateRef(Args&& ... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    using byte = uint8_t;

} // namespace vyra
