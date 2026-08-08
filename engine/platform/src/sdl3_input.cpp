#include "vyra/platform/input.hpp"
#include <SDL3/SDL.h>

namespace vyra {

    bool Input::IsKeyPressed(KeyCode key) {
        int numKeys = 0;
        const bool* state = SDL_GetKeyboardState(&numKeys);
        if (!state) return false;

        SDL_Scancode scancode = SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(key), nullptr);
        if (scancode >= 0 && scancode < numKeys) {
            return state[scancode];
        }
        return false;
    }

    bool Input::IsMouseButtonPressed(MouseButton button) {
        float x = 0.0f, y = 0.0f;
        SDL_MouseButtonFlags flags = SDL_GetMouseState(&x, &y);
        uint32_t mask = 0;
        switch (button) {
            case MouseButton::Left:   mask = SDL_BUTTON_LMASK; break;
            case MouseButton::Right:  mask = SDL_BUTTON_RMASK; break;
            case MouseButton::Middle: mask = SDL_BUTTON_MMASK; break;
            case MouseButton::Button4: mask = SDL_BUTTON_X1MASK; break;
            case MouseButton::Button5: mask = SDL_BUTTON_X2MASK; break;
        }
        return (flags & mask) != 0;
    }

    std::pair<float, float> Input::GetMousePosition() {
        float x = 0.0f, y = 0.0f;
        SDL_GetMouseState(&x, &y);
        return { x, y };
    }

    float Input::GetMouseX() {
        return GetMousePosition().first;
    }

    float Input::GetMouseY() {
        return GetMousePosition().second;
    }

} // namespace vyra
