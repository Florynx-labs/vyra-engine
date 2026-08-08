#pragma once

#include <cstdint>

namespace vyra {

    enum class MouseButton : uint8_t {
        Left     = 1,
        Right    = 2,
        Middle   = 3,
        Button4  = 4,
        Button5  = 5,

        ButtonLast   = Button5,
        ButtonLeft   = Left,
        ButtonRight  = Right,
        ButtonMiddle = Middle
    };

} // namespace vyra
