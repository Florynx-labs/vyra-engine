#include "vyra/profiler/profiler.hpp"
#include "vyra/core/log.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace vyra {
namespace profiler {

Profiler& Profiler::GetInstance() {
    static Profiler instance;
    return instance;
}

Profiler::Profiler()
    : m_enabled(false), m_frameNumber(0) {
}

Profiler::~Profiler() {
    Shutdown();
}

void Profiler::Init() {
    m_enabled = true;
    m_frameNumber = 0;
    m_frameStartTime = std::chrono::high_resolution_clock::now();
    
    VYRA_LOG_CHANNEL(LogChannel::Profiler, info, "Profiler initialized");
}

void Profiler::Shutdown() {
    m_enabled = false;
    VYRA_LOG_CHANNEL(LogChannel::Profiler, info, "Profiler shut down");
}

void Profiler::BeginFrame() {
    if (!m_enabled) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_frameStartTime = std::chrono::high_resolution_clock::now();
    m_currentFrame = FrameProfile();
    m_currentFrame.frameNumber = m_frameNumber.fetch_add(1);
}

void Profiler::EndFrame() {
    if (!m_enabled) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto frameEndTime = std::chrono::high_resolution_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        frameEndTime - m_frameStartTime);
    m_currentFrame.frameTimeMs = frameDuration.count() / 1000.0;
    
    UpdateFrameStatistics();
    CalculatePercentages();
}

void Profiler::RegisterScope(const std::string& name, ProfileCategory category) {
    if (!m_enabled) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_scopeCategories[name] = category;
}

void Profiler::RecordScope(const std::string& name, uint64_t durationUs) {
    if (!m_enabled) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto& stats = m_currentFrame.scopes[name];
    stats.Update(durationUs);
}

const FrameProfile& Profiler::GetCurrentFrame() const {
    return m_currentFrame;
}

const MemoryProfile& Profiler::GetMemoryProfile() const {
    return m_memoryProfile;
}

void Profiler::SetEnabled(bool enabled) {
    m_enabled = enabled;
}

bool Profiler::IsEnabled() const {
    return m_enabled;
}

void Profiler::Reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_currentFrame = FrameProfile();
    m_memoryProfile = MemoryProfile();
    m_frameNumber = 0;
    m_scopeCategories.clear();
}

std::string Profiler::GetFrameProfileString() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "Frame " << m_currentFrame.frameNumber << " - " << m_currentFrame.frameTimeMs << " ms\n";
    ss << std::string(40, '-') << "\n";
    
    // Sort scopes by duration
    std::vector<std::pair<std::string, ProfileStatistics>> sortedScopes;
    for (const auto& [name, stats] : m_currentFrame.scopes) {
        sortedScopes.push_back({name, stats});
    }
    
    std::sort(sortedScopes.begin(), sortedScopes.end(),
        [](const auto& a, const auto& b) {
            return a.second.totalDurationUs > b.second.totalDurationUs;
        });
    
    for (const auto& [name, stats] : sortedScopes) {
        double durationMs = stats.totalDurationUs / 1000.0;
        ss << std::left << std::setw(20) << name << " " 
           << std::right << std::setw(8) << durationMs << " ms "
           << "(" << std::setw(5) << std::fixed << std::setprecision(1) 
           << stats.percentage << "%)\n";
    }
    
    return ss.str();
}

void Profiler::RecordAllocation(size_t size) {
    if (!m_enabled) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_memoryProfile.totalAllocated += size;
    m_memoryProfile.currentUsage += size;
    m_memoryProfile.allocationCount++;
    
    if (m_memoryProfile.currentUsage > m_memoryProfile.peakUsage) {
        m_memoryProfile.peakUsage = m_memoryProfile.currentUsage;
    }
}

void Profiler::RecordFree(size_t size) {
    if (!m_enabled) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_memoryProfile.totalFreed += size;
    m_memoryProfile.currentUsage -= size;
    m_memoryProfile.freeCount++;
}

void Profiler::UpdateFrameStatistics() {
    // Statistics are already updated in RecordScope
}

void Profiler::CalculatePercentages() {
    if (m_currentFrame.frameTimeMs > 0.0) {
        uint64_t frameTimeUs = static_cast<uint64_t>(m_currentFrame.frameTimeMs * 1000.0);
        
        for (auto& [name, stats] : m_currentFrame.scopes) {
            stats.percentage = (stats.totalDurationUs * 100.0) / frameTimeUs;
        }
    }
}

ProfileScopeRAII::ProfileScopeRAII(const std::string& name, ProfileCategory category)
    : m_name(name), m_category(category) {
    m_start = std::chrono::high_resolution_clock::now();
    
    if (Profiler::GetInstance().IsEnabled()) {
        Profiler::GetInstance().RegisterScope(name, category);
    }
}

ProfileScopeRAII::~ProfileScopeRAII() {
    if (Profiler::GetInstance().IsEnabled()) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);
        Profiler::GetInstance().RecordScope(m_name, duration.count());
    }
}

} // namespace profiler
} // namespace vyra
