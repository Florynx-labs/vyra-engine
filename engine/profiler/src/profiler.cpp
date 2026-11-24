#include "vyra/profiler/profiler.hpp"
#include "vyra/core/log.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace vyra::profiler {

Profiler& Profiler::GetInstance() {
    static Profiler instance;
    return instance;
}

Profiler::~Profiler() {
    Shutdown();
}

void Profiler::Init() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_frameNumber = 0;
        m_currentFrame = FrameProfile{};
        m_memoryProfile = MemoryProfile{};
        m_frameStartTime = std::chrono::steady_clock::now();
    }
    m_enabled = true;
    VYRA_LOG_CHANNEL(LogChannel::Profiler, info, "Profiler initialized");
}

void Profiler::Shutdown() {
    m_enabled = false;
}

void Profiler::BeginFrame() {
    if (!m_enabled.load()) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_frameStartTime = std::chrono::steady_clock::now();
    m_currentFrame = FrameProfile{};
    m_currentFrame.frameNumber = m_frameNumber.fetch_add(1);
}

void Profiler::EndFrame() {
    if (!m_enabled.load()) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto end = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        end - m_frameStartTime);
    m_currentFrame.frameTimeMs = static_cast<double>(elapsed.count()) / 1000.0;
    CalculatePercentages();
}

void Profiler::RegisterScope(const std::string& name, ProfileCategory category) {
    if (!m_enabled.load()) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_scopeCategories[name] = category;
}

void Profiler::RecordScope(const std::string& name, uint64_t durationUs) {
    if (!m_enabled.load()) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_currentFrame.scopes[name].Update(durationUs);
}

FrameProfile Profiler::GetCurrentFrame() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentFrame;
}

MemoryProfile Profiler::GetMemoryProfile() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_memoryProfile;
}

void Profiler::SetEnabled(bool enabled) {
    m_enabled = enabled;
}

bool Profiler::IsEnabled() const {
    return m_enabled.load();
}

void Profiler::Reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_currentFrame = FrameProfile{};
    m_memoryProfile = MemoryProfile{};
    m_frameNumber = 0;
    m_scopeCategories.clear();
    m_frameStartTime = std::chrono::steady_clock::now();
}

std::string Profiler::GetFrameProfileString() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2)
           << "Frame " << m_currentFrame.frameNumber << " - "
           << m_currentFrame.frameTimeMs << " ms\n"
           << std::string(40, '-') << "\n";

    std::vector<std::pair<std::string, ProfileStatistics>> scopes(
        m_currentFrame.scopes.begin(), m_currentFrame.scopes.end());
    std::sort(scopes.begin(), scopes.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.second.totalDurationUs > rhs.second.totalDurationUs;
    });

    for (const auto& [name, stats] : scopes) {
        stream << std::left << std::setw(20) << name << " "
               << std::right << std::setw(8)
               << (static_cast<double>(stats.totalDurationUs) / 1000.0)
               << " ms (" << std::setw(5) << std::setprecision(1)
               << stats.percentage << "%)\n";
    }
    return stream.str();
}

void Profiler::RecordAllocation(size_t size) {
    if (!m_enabled.load()) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto bytes = static_cast<uint64_t>(size);
    m_memoryProfile.totalAllocated += bytes;
    m_memoryProfile.currentUsage += bytes;
    ++m_memoryProfile.allocationCount;
    m_memoryProfile.peakUsage = std::max(m_memoryProfile.peakUsage,
                                         m_memoryProfile.currentUsage);
}

void Profiler::RecordFree(size_t size) {
    if (!m_enabled.load()) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto bytes = static_cast<uint64_t>(size);
    // Clamp invalid accounting instead of allowing unsigned underflow.
    const auto released = std::min(bytes, m_memoryProfile.currentUsage);
    m_memoryProfile.totalFreed += released;
    m_memoryProfile.currentUsage -= released;
    ++m_memoryProfile.freeCount;
    if (released != bytes) {
        VYRA_LOG_CHANNEL(LogChannel::Profiler, warn,
                         "Profiler ignored an unmatched free of {0} bytes", bytes - released);
    }
}

void Profiler::CalculatePercentages() {
    const auto frameTimeUs = m_currentFrame.frameTimeMs * 1000.0;
    if (frameTimeUs <= 0.0) return;
    for (auto& [name, stats] : m_currentFrame.scopes) {
        (void)name;
        stats.percentage =
            (static_cast<double>(stats.totalDurationUs) * 100.0) / frameTimeUs;
    }
}

ProfileScopeRAII::ProfileScopeRAII(const std::string& name, ProfileCategory category)
    : m_name(name), m_start(std::chrono::steady_clock::now()) {
    auto& profiler = Profiler::GetInstance();
    if (profiler.IsEnabled()) profiler.RegisterScope(name, category);
}

ProfileScopeRAII::~ProfileScopeRAII() {
    auto& profiler = Profiler::GetInstance();
    if (!profiler.IsEnabled()) return;
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - m_start);
    profiler.RecordScope(m_name, static_cast<uint64_t>(elapsed.count()));
}

} // namespace vyra::profiler
