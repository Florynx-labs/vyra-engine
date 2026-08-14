#pragma once

#include "vyra/core/base.hpp"
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

namespace vyra {
namespace profiler {

/**
 * @brief Profiling category for organizing different types of profiling data
 */
enum class ProfileCategory : uint8_t {
    CPU,
    GPU,
    Frame,
    ECS,
    Memory,
    Asset,
    Other
};

/**
 * @brief Profile scope data for a single measurement
 */
struct ProfileScope {
    std::string name;
    ProfileCategory category;
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    std::chrono::microseconds duration;
    uint32_t depth;
    
    ProfileScope(const std::string& n, ProfileCategory cat, uint32_t d)
        : name(n), category(cat), depth(d) {
        start = std::chrono::high_resolution_clock::now();
    }
    
    void End() {
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    }
};

/**
 * @brief Statistics for a profiled scope
 */
struct ProfileStatistics {
    uint64_t totalDurationUs;       // Total duration in microseconds
    uint64_t averageDurationUs;     // Average duration in microseconds
    uint64_t minDurationUs;        // Minimum duration in microseconds
    uint64_t maxDurationUs;        // Maximum duration in microseconds
    uint32_t callCount;            // Number of times this scope was called
    double percentage;             // Percentage of parent scope or frame
    
    ProfileStatistics()
        : totalDurationUs(0), averageDurationUs(0), minDurationUs(UINT64_MAX),
          maxDurationUs(0), callCount(0), percentage(0.0) {}
    
    void Update(uint64_t durationUs) {
        totalDurationUs += durationUs;
        callCount++;
        
        if (durationUs < minDurationUs) minDurationUs = durationUs;
        if (durationUs > maxDurationUs) maxDurationUs = durationUs;
        
        averageDurationUs = totalDurationUs / callCount;
    }
};

/**
 * @brief Frame profiling data
 */
struct FrameProfile {
    uint64_t frameNumber;
    double frameTimeMs;
    std::unordered_map<std::string, ProfileStatistics> scopes;
    
    FrameProfile() : frameNumber(0), frameTimeMs(0.0) {}
};

/**
 * @brief Memory profiling data
 */
struct MemoryProfile {
    size_t totalAllocated;
    size_t totalFreed;
    size_t currentUsage;
    size_t peakUsage;
    uint32_t allocationCount;
    uint32_t freeCount;
    
    MemoryProfile()
        : totalAllocated(0), totalFreed(0), currentUsage(0),
          peakUsage(0), allocationCount(0), freeCount(0) {}
};

/**
 * @brief Main profiler class
 */
class VYRA_API Profiler {
public:
    static Profiler& GetInstance();
    
    /**
     * @brief Initialize the profiler
     */
    void Init();
    
    /**
     * @brief Shutdown the profiler
     */
    void Shutdown();
    
    /**
     * @brief Begin a new frame
     */
    void BeginFrame();
    
    /**
     * @brief End the current frame
     */
    void EndFrame();
    
    /**
     * @brief Register a profile scope
     */
    void RegisterScope(const std::string& name, ProfileCategory category);
    
    /**
     * @brief Record scope duration
     */
    void RecordScope(const std::string& name, uint64_t durationUs);
    
    /**
     * @brief Get current frame profile
     */
    const FrameProfile& GetCurrentFrame() const;
    
    /**
     * @brief Get memory profile
     */
    const MemoryProfile& GetMemoryProfile() const;
    
    /**
     * @brief Enable/disable profiling
     */
    void SetEnabled(bool enabled);
    
    /**
     * @brief Check if profiling is enabled
     */
    bool IsEnabled() const;
    
    /**
     * @brief Reset all statistics
     */
    void Reset();
    
    /**
     * @brief Get formatted frame profile as string
     */
    std::string GetFrameProfileString() const;
    
    /**
     * @brief Record memory allocation
     */
    void RecordAllocation(size_t size);
    
    /**
     * @brief Record memory free
     */
    void RecordFree(size_t size);

private:
    Profiler();
    ~Profiler();
    
    void UpdateFrameStatistics();
    void CalculatePercentages();
    
    std::atomic<bool> m_enabled;
    std::atomic<uint64_t> m_frameNumber;
    
    FrameProfile m_currentFrame;
    MemoryProfile m_memoryProfile;
    
    std::unordered_map<std::string, ProfileCategory> m_scopeCategories;
    mutable std::mutex m_mutex;
    
    std::chrono::high_resolution_clock::time_point m_frameStartTime;
};

/**
 * @brief RAII scope profiler
 */
class VYRA_API ProfileScopeRAII {
public:
    ProfileScopeRAII(const std::string& name, ProfileCategory category);
    ~ProfileScopeRAII();
    
private:
    std::string m_name;
    ProfileCategory m_category;
    std::chrono::high_resolution_clock::time_point m_start;
};

} // namespace profiler
} // namespace vyra

// Profiling macros
#define VYRA_PROFILE_SCOPE(name, category) \
    vyra::profiler::ProfileScopeRAII VYRA_CONCAT(_vyra_scope_, __LINE__)(name, category)

#define VYRA_PROFILE_FUNCTION() \
    VYRA_PROFILE_SCOPE(__FUNCTION__, vyra::profiler::ProfileCategory::CPU)

#define VYRA_PROFILE_SCOPE_CPU(name) \
    VYRA_PROFILE_SCOPE(name, vyra::profiler::ProfileCategory::CPU)

#define VYRA_PROFILE_SCOPE_GPU(name) \
    VYRA_PROFILE_SCOPE(name, vyra::profiler::ProfileCategory::GPU)

#define VYRA_PROFILE_SCOPE_FRAME(name) \
    VYRA_PROFILE_SCOPE(name, vyra::profiler::ProfileCategory::Frame)

#define VYRA_PROFILE_SCOPE_ECS(name) \
    VYRA_PROFILE_SCOPE(name, vyra::profiler::ProfileCategory::ECS)

#define VYRA_PROFILE_SCOPE_MEMORY(name) \
    VYRA_PROFILE_SCOPE(name, vyra::profiler::ProfileCategory::Memory)

#define VYRA_PROFILE_SCOPE_ASSET(name) \
    VYRA_PROFILE_SCOPE(name, vyra::profiler::ProfileCategory::Asset)

// Helper macro for concatenation
#define VYRA_CONCAT_IMPL(x, y) x##y
#define VYRA_CONCAT(x, y) VYRA_CONCAT_IMPL(x, y)
