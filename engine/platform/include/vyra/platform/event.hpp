#pragma once

#include "vyra/core/base.hpp"
#include "vyra/platform/key_codes.hpp"
#include "vyra/platform/mouse_codes.hpp"

#include <string>
#include <functional>
#include <format>

namespace vyra {

    enum class EventType {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    enum EventCategory {
        None = 0,
        EventCategoryApplication    = (1 << 0),
        EventCategoryInput          = (1 << 1),
        EventCategoryKeyboard       = (1 << 2),
        EventCategoryMouse          = (1 << 3),
        EventCategoryMouseButton    = (1 << 4)
    };

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
                                virtual EventType GetEventType() const override { return GetStaticType(); }\
                                virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

    class VYRA_API Event {
    public:
        virtual ~Event() = default;

        bool Handled = false;

        virtual EventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual int GetCategoryFlags() const = 0;
        virtual std::string ToString() const { return GetName(); }

        bool IsInCategory(EventCategory category) const {
            return (GetCategoryFlags() & category) != 0;
        }
    };

    class EventDispatcher {
    public:
        EventDispatcher(Event& event)
            : m_Event(event) {}

        template<typename T, typename F>
        bool Dispatch(const F& func) {
            if (m_Event.GetEventType() == T::GetStaticType()) {
                m_Event.Handled |= func(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }

    private:
        Event& m_Event;
    };

    // --- Window Events ---
    class VYRA_API WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(uint32_t width, uint32_t height)
            : m_Width(width), m_Height(height) {}

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

        std::string ToString() const override {
            return std::format("WindowResizeEvent: {}, {}", m_Width, m_Height);
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    private:
        uint32_t m_Width, m_Height;
    };

    class VYRA_API WindowCloseEvent : public Event {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class VYRA_API WindowFocusEvent : public Event {
    public:
        WindowFocusEvent(bool focused) : m_Focused(focused) {}
        bool IsFocused() const { return m_Focused; }

        EVENT_CLASS_TYPE(WindowFocus)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    private:
        bool m_Focused;
    };

    // --- Key Events ---
    class VYRA_API KeyEvent : public Event {
    public:
        KeyCode GetKeyCode() const { return m_KeyCode; }

        EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
    protected:
        KeyEvent(KeyCode keycode) : m_KeyCode(keycode) {}
        KeyCode m_KeyCode;
    };

    class VYRA_API KeyPressedEvent : public KeyEvent {
    public:
        KeyPressedEvent(KeyCode keycode, bool isRepeat = false)
            : KeyEvent(keycode), m_IsRepeat(isRepeat) {}

        bool IsRepeat() const { return m_IsRepeat; }

        std::string ToString() const override {
            return std::format("KeyPressedEvent: {} (repeat={})", static_cast<uint16_t>(m_KeyCode), m_IsRepeat);
        }

        EVENT_CLASS_TYPE(KeyPressed)
    private:
        bool m_IsRepeat;
    };

    class VYRA_API KeyReleasedEvent : public KeyEvent {
    public:
        KeyReleasedEvent(KeyCode keycode) : KeyEvent(keycode) {}

        std::string ToString() const override {
            return std::format("KeyReleasedEvent: {}", static_cast<uint16_t>(m_KeyCode));
        }

        EVENT_CLASS_TYPE(KeyReleased)
    };

    // --- Mouse Events ---
    class VYRA_API MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

        float GetX() const { return m_MouseX; }
        float GetY() const { return m_MouseY; }

        std::string ToString() const override {
            return std::format("MouseMovedEvent: {}, {}", m_MouseX, m_MouseY);
        }

        EVENT_CLASS_TYPE(MouseMoved)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
    private:
        float m_MouseX, m_MouseY;
    };

    class VYRA_API MouseScrolledEvent : public Event {
    public:
        MouseScrolledEvent(float xOffset, float yOffset)
            : m_XOffset(xOffset), m_YOffset(yOffset) {}

        float GetXOffset() const { return m_XOffset; }
        float GetYOffset() const { return m_YOffset; }

        std::string ToString() const override {
            return std::format("MouseScrolledEvent: {}, {}", m_XOffset, m_YOffset);
        }

        EVENT_CLASS_TYPE(MouseScrolled)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
    private:
        float m_XOffset, m_YOffset;
    };

    class VYRA_API MouseButtonEvent : public Event {
    public:
        MouseButton GetMouseButton() const { return m_Button; }

        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)
    protected:
        MouseButtonEvent(MouseButton button) : m_Button(button) {}
        MouseButton m_Button;
    };

    class VYRA_API MouseButtonPressedEvent : public MouseButtonEvent {
    public:
        MouseButtonPressedEvent(MouseButton button) : MouseButtonEvent(button) {}

        std::string ToString() const override {
            return std::format("MouseButtonPressedEvent: {}", static_cast<uint8_t>(m_Button));
        }

        EVENT_CLASS_TYPE(MouseButtonPressed)
    };

    class VYRA_API MouseButtonReleasedEvent : public MouseButtonEvent {
    public:
        MouseButtonReleasedEvent(MouseButton button) : MouseButtonEvent(button) {}

        std::string ToString() const override {
            return std::format("MouseButtonReleasedEvent: {}", static_cast<uint8_t>(m_Button));
        }

        EVENT_CLASS_TYPE(MouseButtonReleased)
    };

} // namespace vyra
