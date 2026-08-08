#pragma once

#include "vyra/core/base.hpp"
#include "vyra/platform/event.hpp"

#include <string>
#include <functional>

namespace vyra {

    struct WindowProps {
        std::string Title;
        uint32_t Width;
        uint32_t Height;
        bool VSync;
        bool Fullscreen;

        WindowProps(const std::string& title = "VYRA Engine",
                    uint32_t width = 1600,
                    uint32_t height = 900,
                    bool vsync = true,
                    bool fullscreen = false)
            : Title(title), Width(width), Height(height), VSync(vsync), Fullscreen(fullscreen) {}
    };

    class VYRA_API Window {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() = default;

        virtual void OnUpdate() = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
        virtual void SetVSync(bool enabled) = 0;
        virtual bool IsVSync() const = 0;

        virtual void* GetNativeWindow() const = 0;

        static Scope<Window> Create(const WindowProps& props = WindowProps());
    };

} // namespace vyra
