#include "vyra/core/log.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <vector>

namespace vyra {

    Ref<spdlog::logger> Log::s_CoreLogger;
    Ref<spdlog::logger> Log::s_ClientLogger;
    Ref<spdlog::logger> Log::s_RendererLogger;
    Ref<spdlog::logger> Log::s_ECSLogger;
    Ref<spdlog::logger> Log::s_PhysicsLogger;
    Ref<spdlog::logger> Log::s_AssetsLogger;
    Ref<spdlog::logger> Log::s_EditorLogger;
    Ref<spdlog::logger> Log::s_AILogger;
    Ref<spdlog::logger> Log::s_PlatformLogger;
    Ref<spdlog::logger> Log::s_SceneLogger;
    Ref<spdlog::logger> Log::s_SerializationLogger;

    void Log::Init() {
        std::vector<spdlog::sink_ptr> logSinks;
        logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("vyra_engine.log", true));

        logSinks[0]->set_pattern("%^[%T] [%n] [%l]: %v%$");
        logSinks[1]->set_pattern("[%T] [%l] [%n]: %v");

        s_CoreLogger = std::make_shared<spdlog::logger>("CORE", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_CoreLogger);
        s_CoreLogger->set_level(spdlog::level::trace);
        s_CoreLogger->flush_on(spdlog::level::trace);

        s_ClientLogger = std::make_shared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_ClientLogger);
        s_ClientLogger->set_level(spdlog::level::trace);
        s_ClientLogger->flush_on(spdlog::level::trace);

        s_RendererLogger = std::make_shared<spdlog::logger>("RENDERER", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_RendererLogger);
        s_RendererLogger->set_level(spdlog::level::trace);

        s_ECSLogger = std::make_shared<spdlog::logger>("ECS", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_ECSLogger);
        s_ECSLogger->set_level(spdlog::level::trace);

        s_PhysicsLogger = std::make_shared<spdlog::logger>("PHYSICS", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_PhysicsLogger);
        s_PhysicsLogger->set_level(spdlog::level::trace);

        s_AssetsLogger = std::make_shared<spdlog::logger>("ASSETS", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_AssetsLogger);
        s_AssetsLogger->set_level(spdlog::level::trace);

        s_EditorLogger = std::make_shared<spdlog::logger>("EDITOR", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_EditorLogger);
        s_EditorLogger->set_level(spdlog::level::trace);

        s_AILogger = std::make_shared<spdlog::logger>("AI", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_AILogger);
        s_AILogger->set_level(spdlog::level::trace);

        s_PlatformLogger = std::make_shared<spdlog::logger>("PLATFORM", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_PlatformLogger);
        s_PlatformLogger->set_level(spdlog::level::trace);

        s_SceneLogger = std::make_shared<spdlog::logger>("SCENE", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_SceneLogger);
        s_SceneLogger->set_level(spdlog::level::trace);

        s_SerializationLogger = std::make_shared<spdlog::logger>("SERIALIZATION", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_SerializationLogger);
        s_SerializationLogger->set_level(spdlog::level::trace);
    }

    void Log::Shutdown() {
        spdlog::shutdown();
    }

    Ref<spdlog::logger>& Log::GetLogger(LogChannel channel) {
        if (!s_CoreLogger) {
            Init();
        }
        switch (channel) {
            case LogChannel::Core:         return s_CoreLogger;
            case LogChannel::Renderer:      return s_RendererLogger;
            case LogChannel::ECS:           return s_ECSLogger;
            case LogChannel::Physics:       return s_PhysicsLogger;
            case LogChannel::Assets:        return s_AssetsLogger;
            case LogChannel::Editor:        return s_EditorLogger;
            case LogChannel::AI:            return s_AILogger;
            case LogChannel::Platform:      return s_PlatformLogger;
            case LogChannel::Scene:         return s_SceneLogger;
            case LogChannel::Serialization: return s_SerializationLogger;
            default:                        return s_CoreLogger;
        }
    }

} // namespace vyra