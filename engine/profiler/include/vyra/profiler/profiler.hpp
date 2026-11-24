#pragma once

#include "vyra/core/base.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace vyra::profiler {

enum class ProfileCategory : uint8_t { CPU, GPU, Frame, ECS, Memory, Asset, Other };

struct ProfileStatistics {
    uint64_t totalDurationUs{0};
    uint64_t averageDurationUs{0};
    uint64_t minDurationUs{UINT64_MAX};
    uint64_t maxDurationUs{0};
    uint64_t callCount{0};
    double percentage{0.0};

    void Update(uint64_t durationUs) noexcept {
        totalDurationUs += durationUs;
        ++callCount;
        if (durationUs < minDurationUs) minDurationUs = durationUs;
        if (durationUs > maxDurationUs) maxDurationUs = durationUs;
        averageDurationUs = totalDurationUs / callCount;
    }
};

struct FrameProfile {
    uint64_t frameNumber{0};
    double frameTimeMs{0.0};
    std::unordered_map<std::string, ProfileStatistics> scopes;
};

struct MemoryProfile {
    uint64_t totalAllocated{0};
    uint64_t totalFreed{0};
    uint64_t currentUsage{0};
    uint64_t peakUsage{0};
    uint64_t allocationCount{0};
    uint64_t freeCount{0};
};

class VYRA_API Profiler {
public:
    static Profiler& GetInstance();
    void Init();
    void Shutdown();
    void BeginFrame();
    void EndFrame();
    void RegisterScope(const std::string& name, ProfileCategory category);
    void RecordScope(const std::string& name, uint64_t durationUs);
    FrameProfile GetCurrentFrame() const;
    MemoryProfile GetMemoryProfile() const;
    void SetEnabled(bool enabled);
    bool IsEnabled() const;
    void Reset();
    std::string GetFrameProfileString() const;
    void RecordAllocation(size_t size);
    void RecordFree(size_t size);

private:
    Profiler() = default;
    ~Profiler();
    void CalculatePercentages();

    std::atomic<bool> m_enabled{false};
    std::atomic<uint64_t> m_frameNumber{0};
    FrameProfile m_currentFrame;
    MemoryProfile m_memoryProfile;
    std::unordered_map<std::string, ProfileCategory> m_scopeCategories;
    mutable std::mutex m_mutex;
    std::chrono::steady_clock::time_point m_frameStartTime{};
};

class VYRA_API ProfileScopeRAII {
public:
    ProfileScopeRAII(const std::string& name, ProfileCategory category);
    ~ProfileScopeRAII();
    ProfileScopeRAII(const ProfileScopeRAII&) = delete;
    ProfileScopeRAII& operator=(const ProfileScopeRAII&) = delete;
private:
    std::string m_name;
    std::chrono::steady_clock::time_point m_start;
};

} // namespace vyra::profiler

#define VYRA_PROFILE_SCOPE(name, category) \
    ::vyra::profiler::ProfileScopeRAII VYRA_CONCAT(_vyra_scope_, __LINE__)(name, category)
#define VYRA_PROFILE_FUNCTION() \
    VYRA_PROFILE_SCOPE(__FUNCTION__, ::vyra::profiler::ProfileCategory::CPU)
#define VYRA_PROFILE_SCOPE_CPU(name) \
    VYRA_PROFILE_SCOPE(name, ::vyra::profiler::ProfileCategory::CPU)
#define VYRA_PROFILE_SCOPE_GPU(name) \
    VYRA_PROFILE_SCOPE(name, ::vyra::profiler::ProfileCategory::GPU)
#define VYRA_PROFILE_SCOPE_FRAME(name) \
    VYRA_PROFILE_SCOPE(name, ::vyra::profiler::ProfileCategory::Frame)
#define VYRA_PROFILE_SCOPE_ECS(name) \
    VYRA_PROFILE_SCOPE(name, ::vyra::profiler::ProfileCategory::ECS)
#define VYRA_PROFILE_SCOPE_MEMORY(name) \
    VYRA_PROFILE_SCOPE(name, ::vyra::profiler::ProfileCategory::Memory)
#define VYRA_PROFILE_SCOPE_ASSET(name) \
    VYRA_PROFILE_SCOPE(name, ::vyra::profiler::ProfileCategory::Asset)
#define VYRA_CONCAT_IMPL(x, y) x##y
#define VYRA_CONCAT(x, y) VYRA_CONCAT_IMPL(x, y)
