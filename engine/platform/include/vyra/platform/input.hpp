#pragma once

#include "vyra/core/base.hpp"
#include "vyra/platform/key_codes.hpp"
#include "vyra/platform/mouse_codes.hpp"
#include <utility>

namespace vyra {

    class VYRA_API Input {
    public:
        static bool IsKeyPressed(KeyCode key);
        static bool IsMouseButtonPressed(MouseButton button);
        static std::pair<float, float> GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();
    };

} // namespace vyra
