#pragma once

#include "vyra/platform/window.hpp"
#include <SDL3/SDL.h>

namespace vyra {

    class SDL3Window : public Window {
    public:
        SDL3Window(const WindowProps& props);
        virtual ~SDL3Window();

        void OnUpdate() override;

        uint32_t GetWidth() const override { return m_Data.Width; }
        uint32_t GetHeight() const override { return m_Data.Height; }

        void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
        void SetVSync(bool enabled) override;
        bool IsVSync() const override { return m_Data.VSync; }

        void* GetNativeWindow() const override { return m_Window; }

    private:
        virtual void Init(const WindowProps& props);
        virtual void Shutdown();
        void ProcessSDLEvent(const SDL_Event& sdlEvent);

    private:
        SDL_Window* m_Window = nullptr;

        struct WindowData {
            std::string Title;
            uint32_t Width = 0;
            uint32_t Height = 0;
            bool VSync = true;

            EventCallbackFn EventCallback;
        };

        WindowData m_Data;
    };

} // namespace vyra
