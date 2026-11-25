#pragma once

#include "vyra/core/base.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>

namespace vyra::input {

enum class ControllerAxis : uint8_t { LeftX, LeftY, RightX, RightY, LeftTrigger, RightTrigger };

struct ControllerState {
    std::unordered_map<ControllerAxis, float> Axes;
    std::unordered_map<uint32_t, bool> Buttons;
};

class VYRA_API ControllerMapper {
public:
    bool SetDeadzone(ControllerAxis axis, float deadzone) {
        if (!std::isfinite(deadzone) || deadzone < 0.0f || deadzone >= 1.0f) return false;
        m_Deadzones[axis] = deadzone; return true;
    }
    float GetAxis(const ControllerState& state, ControllerAxis axis) const {
        const auto it = state.Axes.find(axis);
        if (it == state.Axes.end() || !std::isfinite(it->second)) return 0.0f;
        const float raw = std::clamp(it->second, -1.0f, 1.0f);
        const auto dz = m_Deadzones.find(axis);
        const float deadzone = dz == m_Deadzones.end() ? 0.15f : dz->second;
        if (std::abs(raw) <= deadzone) return 0.0f;
        const float scaled = (std::abs(raw) - deadzone) / (1.0f - deadzone);
        return std::copysign(scaled, raw);
    }
    bool IsButtonDown(const ControllerState& state, uint32_t button) const {
        const auto it = state.Buttons.find(button);
        return it != state.Buttons.end() && it->second;
    }
private:
    std::unordered_map<ControllerAxis, float> m_Deadzones;
};

} // namespace vyra::input
