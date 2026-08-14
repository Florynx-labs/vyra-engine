#pragma once

#include "vyra/core/base.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace vyra {

    enum class LogChannel : uint8_t {
        Core,
        Renderer,
        ECS,
        Physics,
        Assets,
        Editor,
        AI,
        Platform,
        Scene,
        Serialization,
        Profiler
    };

    class VYRA_API Log {
    public:
        static void Init();
        static void Shutdown();

        static Ref<spdlog::logger>& GetCoreLogger() {
            if (!s_CoreLogger) Init();
            return s_CoreLogger;
        }
        static Ref<spdlog::logger>& GetClientLogger() {
            if (!s_ClientLogger) Init();
            return s_ClientLogger;
        }
        static Ref<spdlog::logger>& GetLogger(LogChannel channel);

    private:
        static Ref<spdlog::logger> s_CoreLogger;
        static Ref<spdlog::logger> s_ClientLogger;
        static Ref<spdlog::logger> s_RendererLogger;
        static Ref<spdlog::logger> s_ECSLogger;
        static Ref<spdlog::logger> s_PhysicsLogger;
        static Ref<spdlog::logger> s_AssetsLogger;
        static Ref<spdlog::logger> s_EditorLogger;
        static Ref<spdlog::logger> s_AILogger;
        static Ref<spdlog::logger> s_PlatformLogger;
        static Ref<spdlog::logger> s_SceneLogger;
        static Ref<spdlog::logger> s_SerializationLogger;
        static Ref<spdlog::logger> s_ProfilerLogger;
    };

} // namespace vyra

// Core Logging Macros
#define VYRA_LOG_TRACE(...)   ::vyra::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define VYRA_LOG_DEBUG(...)   ::vyra::Log::GetCoreLogger()->debug(__VA_ARGS__)
#define VYRA_LOG_INFO(...)    ::vyra::Log::GetCoreLogger()->info(__VA_ARGS__)
#define VYRA_LOG_WARN(...)    ::vyra::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define VYRA_LOG_ERROR(...)   ::vyra::Log::GetCoreLogger()->error(__VA_ARGS__)
#define VYRA_LOG_CRITICAL(...) ::vyra::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client / App Logging Macros
#define VYRA_TRACE(...)       ::vyra::Log::GetClientLogger()->trace(__VA_ARGS__)
#define VYRA_INFO(...)        ::vyra::Log::GetClientLogger()->info(__VA_ARGS__)
#define VYRA_WARN(...)        ::vyra::Log::GetClientLogger()->warn(__VA_ARGS__)
#define VYRA_ERROR(...)       ::vyra::Log::GetClientLogger()->error(__VA_ARGS__)
#define VYRA_CRITICAL(...)    ::vyra::Log::GetClientLogger()->critical(__VA_ARGS__)

// Channel Specific Logging Macros
#define VYRA_LOG_CHANNEL(channel, severity, ...) \
    ::vyra::Log::GetLogger(channel)->severity(__VA_ARGS__)
