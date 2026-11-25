#pragma once

#include "vyra/core/base.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace vyra::runtime {

using InputCode = uint32_t;

struct ActionState {
    bool down{false};
    bool pressed{false};
    bool released{false};
};

// Backend-neutral action mapping. A platform adapter feeds physical button
// states each frame; gameplay code consumes named actions instead of SDL codes.
class VYRA_API InputActionMap {
public:
    bool Bind(const std::string& action, InputCode code) {
        if (action.empty()) return false;
        auto& bindings = m_Bindings[action];
        for (const auto existing : bindings) if (existing == code) return true;
        bindings.push_back(code);
        m_States.try_emplace(action, ActionState{});
        return true;
    }

    bool Unbind(const std::string& action, InputCode code) {
        const auto it = m_Bindings.find(action);
        if (it == m_Bindings.end()) return false;
        auto& bindings = it->second;
        const auto oldSize = bindings.size();
        std::erase(bindings, code);
        if (bindings.empty()) {
            m_Bindings.erase(it);
            m_States.erase(action);
        }
        return bindings.size() != oldSize;
    }

    void SetButtonDown(InputCode code, bool down) {
        if (down) m_DownButtons.insert(code);
        else m_DownButtons.erase(code);
    }

    // Call exactly once after polling the platform input state for a frame.
    void AdvanceFrame() {
        for (const auto& [action, bindings] : m_Bindings) {
            bool nowDown = false;
            for (const auto code : bindings) {
                if (m_DownButtons.contains(code)) { nowDown = true; break; }
            }
            auto& state = m_States[action];
            state.pressed = nowDown && !state.down;
            state.released = !nowDown && state.down;
            state.down = nowDown;
        }
    }

    ActionState GetState(const std::string& action) const {
        const auto it = m_States.find(action);
        return it == m_States.end() ? ActionState{} : it->second;
    }

    bool IsDown(const std::string& action) const { return GetState(action).down; }
    bool WasPressed(const std::string& action) const { return GetState(action).pressed; }
    bool WasReleased(const std::string& action) const { return GetState(action).released; }

    void Reset() {
        m_DownButtons.clear();
        for (auto& [_, state] : m_States) state = {};
    }

private:
    std::unordered_map<std::string, std::vector<InputCode>> m_Bindings;
    std::unordered_map<std::string, ActionState> m_States;
    std::unordered_set<InputCode> m_DownButtons;
};

} // namespace vyra::runtime
