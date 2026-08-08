#pragma once

#include "vyra/core/base.hpp"
#include <chrono>

namespace vyra {

    class VYRA_API Timestep {
    public:
        Timestep(float time = 0.0f)
            : m_Time(time) {}

        operator float() const { return m_Time; }

        float GetSeconds() const { return m_Time; }
        float GetMilliseconds() const { return m_Time * 1000.0f; }

    private:
        float m_Time;
    };

    class VYRA_API Time {
    public:
        static void Init();
        static float GetTime(); // Returns elapsed time in seconds since engine launch
        static double GetTimeExact();

    private:
        static std::chrono::time_point<std::chrono::high_resolution_clock> s_StartTime;
    };

} // namespace vyra
