#include "vyra/core/time.hpp"

namespace vyra {

    std::chrono::time_point<std::chrono::high_resolution_clock> Time::s_StartTime;

    void Time::Init() {
        s_StartTime = std::chrono::high_resolution_clock::now();
    }

    float Time::GetTime() {
        return static_cast<float>(GetTimeExact());
    }

    double Time::GetTimeExact() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = currentTime - s_StartTime;
        return elapsed.count();
    }

} // namespace vyra
