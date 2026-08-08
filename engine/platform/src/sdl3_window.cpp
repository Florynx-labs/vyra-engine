#include "sdl3_window.hpp"
#include "vyra/core/log.hpp"
#include "vyra/core/assert.hpp"

namespace vyra {

    static bool s_SDLInitialized = false;

    Scope<Window> Window::Create(const WindowProps& props) {
        return CreateScope<SDL3Window>(props);
    }

    static KeyCode SDLKeyToVyraKey(SDL_Keycode key) {
        switch (key) {
            case SDLK_SPACE:        return KeyCode::Space;
            case SDLK_APOSTROPHE:   return KeyCode::Apostrophe;
            case SDLK_COMMA:        return KeyCode::Comma;
            case SDLK_MINUS:        return KeyCode::Minus;
            case SDLK_PERIOD:       return KeyCode::Period;
            case SDLK_SLASH:        return KeyCode::Slash;
            case SDLK_0:            return KeyCode::D0;
            case SDLK_1:            return KeyCode::D1;
            case SDLK_2:            return KeyCode::D2;
            case SDLK_3:            return KeyCode::D3;
            case SDLK_4:            return KeyCode::D4;
            case SDLK_5:            return KeyCode::D5;
            case SDLK_6:            return KeyCode::D6;
            case SDLK_7:            return KeyCode::D7;
            case SDLK_8:            return KeyCode::D8;
            case SDLK_9:            return KeyCode::D9;
            case SDLK_SEMICOLON:    return KeyCode::Semicolon;
            case SDLK_EQUALS:       return KeyCode::Equal;
            case SDLK_A:            return KeyCode::A;
            case SDLK_B:            return KeyCode::B;
            case SDLK_C:            return KeyCode::C;
            case SDLK_D:            return KeyCode::D;
            case SDLK_E:            return KeyCode::E;
            case SDLK_F:            return KeyCode::F;
            case SDLK_G:            return KeyCode::G;
            case SDLK_H:            return KeyCode::H;
            case SDLK_I:            return KeyCode::I;
            case SDLK_J:            return KeyCode::J;
            case SDLK_K:            return KeyCode::K;
            case SDLK_L:            return KeyCode::L;
            case SDLK_M:            return KeyCode::M;
            case SDLK_N:            return KeyCode::N;
            case SDLK_O:            return KeyCode::O;
            case SDLK_P:            return KeyCode::P;
            case SDLK_Q:            return KeyCode::Q;
            case SDLK_R:            return KeyCode::R;
            case SDLK_S:            return KeyCode::S;
            case SDLK_T:            return KeyCode::T;
            case SDLK_U:            return KeyCode::U;
            case SDLK_V:            return KeyCode::V;
            case SDLK_W:            return KeyCode::W;
            case SDLK_X:            return KeyCode::X;
            case SDLK_Y:            return KeyCode::Y;
            case SDLK_Z:            return KeyCode::Z;
            case SDLK_LEFTBRACKET:  return KeyCode::LeftBracket;
            case SDLK_BACKSLASH:    return KeyCode::Backslash;
            case SDLK_RIGHTBRACKET: return KeyCode::RightBracket;
            case SDLK_GRAVE:        return KeyCode::GraveAccent;
            case SDLK_ESCAPE:       return KeyCode::Escape;
            case SDLK_RETURN:       return KeyCode::Enter;
            case SDLK_TAB:          return KeyCode::Tab;
            case SDLK_BACKSPACE:    return KeyCode::Backspace;
            case SDLK_INSERT:       return KeyCode::Insert;
            case SDLK_DELETE:       return KeyCode::Delete;
            case SDLK_RIGHT:        return KeyCode::Right;
            case SDLK_LEFT:         return KeyCode::Left;
            case SDLK_DOWN:         return KeyCode::Down;
            case SDLK_UP:           return KeyCode::Up;
            case SDLK_PAGEUP:       return KeyCode::PageUp;
            case SDLK_PAGEDOWN:     return KeyCode::PageDown;
            case SDLK_HOME:         return KeyCode::Home;
            case SDLK_END:          return KeyCode::End;
            case SDLK_CAPSLOCK:     return KeyCode::CapsLock;
            case SDLK_SCROLLLOCK:   return KeyCode::ScrollLock;
            case SDLK_NUMLOCKCLEAR: return KeyCode::NumLock;
            case SDLK_PRINTSCREEN:  return KeyCode::PrintScreen;
            case SDLK_PAUSE:        return KeyCode::Pause;
            case SDLK_F1:           return KeyCode::F1;
            case SDLK_F2:           return KeyCode::F2;
            case SDLK_F3:           return KeyCode::F3;
            case SDLK_F4:           return KeyCode::F4;
            case SDLK_F5:           return KeyCode::F5;
            case SDLK_F6:           return KeyCode::F6;
            case SDLK_F7:           return KeyCode::F7;
            case SDLK_F8:           return KeyCode::F8;
            case SDLK_F9:           return KeyCode::F9;
            case SDLK_F10:          return KeyCode::F10;
            case SDLK_F11:          return KeyCode::F11;
            case SDLK_F12:          return KeyCode::F12;
            case SDLK_LSHIFT:       return KeyCode::LeftShift;
            case SDLK_LCTRL:        return KeyCode::LeftControl;
            case SDLK_LALT:         return KeyCode::LeftAlt;
            case SDLK_LGUI:         return KeyCode::LeftSuper;
            case SDLK_RSHIFT:       return KeyCode::RightShift;
            case SDLK_RCTRL:        return KeyCode::RightControl;
            case SDLK_RALT:         return KeyCode::RightAlt;
            case SDLK_RGUI:         return KeyCode::RightSuper;
            default:                return KeyCode::Unknown;
        }
    }

    static MouseButton SDLButtonToVyraButton(uint8_t button) {
        switch (button) {
            case SDL_BUTTON_LEFT:   return MouseButton::Left;
            case SDL_BUTTON_RIGHT:  return MouseButton::Right;
            case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
            case SDL_BUTTON_X1:     return MouseButton::Button4;
            case SDL_BUTTON_X2:     return MouseButton::Button5;
            default:                return MouseButton::Left;
        }
    }

    SDL3Window::SDL3Window(const WindowProps& props) {
        Init(props);
    }

    SDL3Window::~SDL3Window() {
        Shutdown();
    }

    void SDL3Window::Init(const WindowProps& props) {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        VYRA_LOG_INFO("Creating SDL3 Window '{0}' ({1}, {2})", props.Title, props.Width, props.Height);

        if (!s_SDLInitialized) {
            bool success = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
            VYRA_CORE_ASSERT(success, "Failed to initialize SDL3: {0}", SDL_GetError());
            s_SDLInitialized = true;
        }

        SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
        if (props.Fullscreen) {
            flags |= SDL_WINDOW_FULLSCREEN;
        }

        m_Window = SDL_CreateWindow(m_Data.Title.c_str(), static_cast<int>(m_Data.Width), static_cast<int>(m_Data.Height), flags);
        VYRA_CORE_ASSERT(m_Window != nullptr, "Failed to create SDL3 window: {0}", SDL_GetError());
    }

    void SDL3Window::Shutdown() {
        if (m_Window) {
            SDL_DestroyWindow(m_Window);
            m_Window = nullptr;
        }
        if (s_SDLInitialized) {
            SDL_Quit();
            s_SDLInitialized = false;
        }
    }

    void SDL3Window::OnUpdate() {
        SDL_Event sdlEvent;
        while (SDL_PollEvent(&sdlEvent)) {
            ProcessSDLEvent(sdlEvent);
        }
    }

    void SDL3Window::ProcessSDLEvent(const SDL_Event& sdlEvent) {
        if (!m_Data.EventCallback) return;

        switch (sdlEvent.type) {
            case SDL_EVENT_QUIT: {
                WindowCloseEvent event;
                m_Data.EventCallback(event);
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
                m_Data.Width = static_cast<uint32_t>(sdlEvent.window.data1);
                m_Data.Height = static_cast<uint32_t>(sdlEvent.window.data2);
                WindowResizeEvent event(m_Data.Width, m_Data.Height);
                m_Data.EventCallback(event);
                break;
            }
            case SDL_EVENT_WINDOW_FOCUS_GAINED: {
                WindowFocusEvent event(true);
                m_Data.EventCallback(event);
                break;
            }
            case SDL_EVENT_WINDOW_FOCUS_LOST: {
                WindowFocusEvent event(false);
                m_Data.EventCallback(event);
                break;
            }
            case SDL_EVENT_KEY_DOWN: {
                KeyCode key = SDLKeyToVyraKey(sdlEvent.key.key);
                KeyPressedEvent event(key, sdlEvent.key.repeat != 0);
                m_Data.EventCallback(event);
                break;
            }
            case SDL_EVENT_KEY_UP: {
                KeyCode key = SDLKeyToVyraKey(sdlEvent.key.key);
                KeyReleasedEvent event(key);
                m_Data.EventCallback(event);
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                MouseButton button = SDLButtonToVyraButton(sdlEvent.button.button);
                MouseButtonPressedEvent event(button);
                m_Data.EventCallback(event);
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                MouseButton button = SDLButtonToVyraButton(sdlEvent.button.button);
                MouseButtonReleasedEvent event(button);
                m_Data.EventCallback(event);
                break;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                MouseMovedEvent event(sdlEvent.motion.x, sdlEvent.motion.y);
                m_Data.EventCallback(event);
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL: {
                MouseScrolledEvent event(sdlEvent.wheel.x, sdlEvent.wheel.y);
                m_Data.EventCallback(event);
                break;
            }
        }
    }

    void SDL3Window::SetVSync(bool enabled) {
        m_Data.VSync = enabled;
    }

} // namespace vyra
