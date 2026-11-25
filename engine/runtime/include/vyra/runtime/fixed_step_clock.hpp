#pragma once

#include "vyra/core/base.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>

namespace vyra::runtime {

struct FixedStepResult {
    uint32_t steps{0};
    double interpolationAlpha{0.0};
    double droppedTimeSeconds{0.0};
};

// Converts variable frame deltas into bounded deterministic simulation ticks.
class VYRA_API FixedStepClock {
public:
    explicit FixedStepClock(double fixedDeltaSeconds = 1.0 / 60.0,
                            uint32_t maxSubsteps = 8,
                            double maxFrameDeltaSeconds = 0.25)
        : m_FixedDelta(fixedDeltaSeconds), m_MaxSubsteps(maxSubsteps),
          m_MaxFrameDelta(maxFrameDeltaSeconds) {
        if (!std::isfinite(m_FixedDelta) || m_FixedDelta <= 0.0)
            m_FixedDelta = 1.0 / 60.0;
        if (m_MaxSubsteps == 0) m_MaxSubsteps = 1;
        if (!std::isfinite(m_MaxFrameDelta) || m_MaxFrameDelta < m_FixedDelta)
            m_MaxFrameDelta = m_FixedDelta * static_cast<double>(m_MaxSubsteps);
    }

    template <typename StepFunction>
    FixedStepResult Advance(double frameDeltaSeconds, StepFunction&& step) {
        FixedStepResult result{};
        if (!std::isfinite(frameDeltaSeconds) || frameDeltaSeconds < 0.0)
            frameDeltaSeconds = 0.0;

        const double accepted = std::min(frameDeltaSeconds, m_MaxFrameDelta);
        result.droppedTimeSeconds = frameDeltaSeconds - accepted;
        m_Accumulator += accepted;

        while (m_Accumulator >= m_FixedDelta && result.steps < m_MaxSubsteps) {
            std::invoke(step, m_FixedDelta);
            m_Accumulator -= m_FixedDelta;
            ++result.steps;
        }

        // Avoid a spiral of death: retain only the fractional remainder after
        // the substep budget is exhausted and report the discarded simulation time.
        if (m_Accumulator >= m_FixedDelta) {
            const double retained = std::fmod(m_Accumulator, m_FixedDelta);
            result.droppedTimeSeconds += m_Accumulator - retained;
            m_Accumulator = retained;
        }

        result.interpolationAlpha = std::clamp(m_Accumulator / m_FixedDelta, 0.0, 1.0);
        return result;
    }

    void Reset() noexcept { m_Accumulator = 0.0; }
    double GetFixedDeltaSeconds() const noexcept { return m_FixedDelta; }
    double GetAccumulatorSeconds() const noexcept { return m_Accumulator; }

private:
    double m_FixedDelta;
    uint32_t m_MaxSubsteps;
    double m_MaxFrameDelta;
    double m_Accumulator{0.0};
};

} // namespace vyra::runtime
