#pragma once

#include "vyra/core/base.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace vyra::runtime {

enum class Interpolation : uint8_t { Step, Linear };

struct ScalarKeyframe {
    double timeSeconds{0.0};
    float value{0.0f};
};

// A deterministic scalar animation channel suitable for material parameters,
// camera values, gameplay properties and as a building block for vector tracks.
class VYRA_API ScalarAnimationTrack {
public:
    bool AddKeyframe(double timeSeconds, float value) {
        if (!std::isfinite(timeSeconds) || timeSeconds < 0.0 ||
            !std::isfinite(value)) return false;

        const auto it = std::lower_bound(m_Keys.begin(), m_Keys.end(), timeSeconds,
            [](const ScalarKeyframe& key, double time) { return key.timeSeconds < time; });
        if (it != m_Keys.end() && it->timeSeconds == timeSeconds) {
            it->value = value;
        } else {
            m_Keys.insert(it, ScalarKeyframe{timeSeconds, value});
        }
        return true;
    }

    float Sample(double timeSeconds, Interpolation interpolation = Interpolation::Linear,
                 bool loop = false) const {
        if (m_Keys.empty()) return 0.0f;
        if (!std::isfinite(timeSeconds)) timeSeconds = 0.0;
        const double duration = GetDuration();
        if (loop && duration > 0.0) {
            timeSeconds = std::fmod(std::max(0.0, timeSeconds), duration);
        } else {
            timeSeconds = std::clamp(timeSeconds, m_Keys.front().timeSeconds, duration);
        }

        const auto upper = std::upper_bound(m_Keys.begin(), m_Keys.end(), timeSeconds,
            [](double time, const ScalarKeyframe& key) { return time < key.timeSeconds; });
        if (upper == m_Keys.begin()) return upper->value;
        if (upper == m_Keys.end()) return m_Keys.back().value;

        const auto& left = *(upper - 1);
        const auto& right = *upper;
        if (interpolation == Interpolation::Step) return left.value;
        const double span = right.timeSeconds - left.timeSeconds;
        if (span <= 0.0) return right.value;
        const float alpha = static_cast<float>((timeSeconds - left.timeSeconds) / span);
        return left.value + (right.value - left.value) * alpha;
    }

    double GetDuration() const noexcept {
        return m_Keys.empty() ? 0.0 : m_Keys.back().timeSeconds;
    }
    size_t GetKeyframeCount() const noexcept { return m_Keys.size(); }
    const std::vector<ScalarKeyframe>& GetKeyframes() const noexcept { return m_Keys; }
    void Clear() noexcept { m_Keys.clear(); }

private:
    std::vector<ScalarKeyframe> m_Keys;
};

class VYRA_API AnimationPlayer {
public:
    void Play(bool restart = false) noexcept {
        if (restart) m_Time = 0.0;
        m_Playing = true;
    }
    void Pause() noexcept { m_Playing = false; }
    void Stop() noexcept { m_Playing = false; m_Time = 0.0; }
    void SetLooping(bool looping) noexcept { m_Looping = looping; }
    bool IsPlaying() const noexcept { return m_Playing; }
    double GetTimeSeconds() const noexcept { return m_Time; }

    void Update(double deltaSeconds, double durationSeconds) noexcept {
        if (!m_Playing || !std::isfinite(deltaSeconds) || deltaSeconds <= 0.0 ||
            !std::isfinite(durationSeconds) || durationSeconds <= 0.0) return;
        m_Time += deltaSeconds;
        if (m_Looping) {
            m_Time = std::fmod(m_Time, durationSeconds);
        } else if (m_Time >= durationSeconds) {
            m_Time = durationSeconds;
            m_Playing = false;
        }
    }

private:
    double m_Time{0.0};
    bool m_Playing{false};
    bool m_Looping{false};
};

} // namespace vyra::runtime
