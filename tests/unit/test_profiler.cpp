#include <catch2/catch_test_macros.hpp>
#include <vyra/profiler/profiler.hpp>
#include <vyra/core/log.hpp>
#include <thread>
#include <chrono>

TEST_CASE("VYRA Profiler - Basic Functionality", "[profiler][unit]") {
    SECTION("Profiler Initialization") {
        auto& profiler = vyra::profiler::Profiler::GetInstance();
        profiler.Init();
        
        REQUIRE(profiler.IsEnabled());
        
        profiler.Shutdown();
    }
    
    SECTION("Profiler Enable/Disable") {
        auto& profiler = vyra::profiler::Profiler::GetInstance();
        profiler.Init();
        
        profiler.SetEnabled(false);
        REQUIRE_FALSE(profiler.IsEnabled());
        
        profiler.SetEnabled(true);
        REQUIRE(profiler.IsEnabled());
        
        profiler.Shutdown();
    }
    
    SECTION("Frame Profiling") {
        auto& profiler = vyra::profiler::Profiler::GetInstance();
        profiler.Init();
        
        profiler.BeginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        profiler.EndFrame();
        
        const auto& frame = profiler.GetCurrentFrame();
        REQUIRE(frame.frameTimeMs >= 10.0); // At least 10ms
        REQUIRE(frame.frameNumber == 0);
        
        profiler.Shutdown();
    }
    
    SECTION("Multiple Frames") {
        auto& profiler = vyra::profiler::Profiler::GetInstance();
        profiler.Init();
        
        for (int i = 0; i < 5; i++) {
            profiler.BeginFrame();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            profiler.EndFrame();
        }
        
        const auto& frame = profiler.GetCurrentFrame();
        REQUIRE(frame.frameNumber == 4); // 0-indexed, so 5th frame is 4
        
        profiler.Shutdown();
    }
    
    SECTION("Scope Registration") {
        auto& profiler = vyra::profiler::Profiler::GetInstance();
        profiler.Init();
        
        profiler.RegisterScope("TestScope", vyra::profiler::ProfileCategory::CPU);
        
        profiler.Shutdown();
    }
    
    SECTION("Scope Recording") {
        auto& profiler = vyra::profiler::Profiler::GetInstance();
        profiler.Init();
        
        profiler.RegisterScope("TestScope", vyra::profiler::ProfileCategory::CPU);
        profiler.RecordScope("TestScope", 1000); // 1ms
        
        const auto& frame = profiler.GetCurrentFrame();
        auto it = frame.scopes.find("TestScope");
        REQUIRE(it != frame.scopes.end());
        REQUIRE(it->second.totalDurationUs == 1000);
        REQUIRE(it->second.callCount == 1);
        
        profiler.Shutdown();
    }
    
    SECTION("Profile Statistics") {
        auto& profiler = vyra::profiler::Profiler::GetInstance();
        profiler.Init();
        profiler.Reset();
        
        profiler.RegisterScope("TestScopeStats", vyra::profiler::ProfileCategory::CPU);
        profiler.RecordScope("TestScopeStats", 1000);
        profiler.RecordScope("TestScopeStats", 2000);
        profiler.RecordScope("TestScopeStats", 3000);
        
        const auto& frame = profiler.GetCurrentFrame();
        auto it = frame.scopes.find("TestScopeStats");
        REQUIRE(it != frame.scopes.end());
        const auto& stats = it->second;
        REQUIRE(stats.totalDurationUs == 6000);
        REQUIRE(stats.callCount == 3);
        REQUIRE(stats.averageDurationUs == 2000);
        REQUIRE(stats.minDurationUs == 1000);
        REQUIRE(stats.maxDurationUs == 3000);
        
        profiler.Shutdown();
    }
    
    SECTION("Memory Profiling") {
        auto& profiler = vyra::profiler::Profiler::GetInstance();
        profiler.Init();
        
        profiler.RecordAllocation(1024);
        profiler.RecordAllocation(2048);
        profiler.RecordFree(512);
        
        const auto& mem = profiler.GetMemoryProfile();
        REQUIRE(mem.totalAllocated == 3072);
        REQUIRE(mem.totalFreed == 512);
        REQUIRE(mem.currentUsage == 2560);
        REQUIRE(mem.allocationCount == 2);
        REQUIRE(mem.freeCount == 1);
        
        profiler.Shutdown();
    }
    
    SECTION("Memory Peak Tracking") {
        auto& profiler = vyra::profiler::Profiler::GetInstance();
        profiler.Init();
        profiler.Reset();
        
        profiler.RecordAllocation(1024);
        profiler.RecordAllocation(2048);
        profiler.RecordFree(1024);
        
        const auto& mem = profiler.GetMemoryProfile();
        REQUIRE(mem.peakUsage == 3072); // Peak at 1024 + 2048
        
        profiler.Shutdown();
    }
    
    SECTION("Profiler Reset") {
        auto& profiler = vyra::profiler::Profiler::GetInstance();
        profiler.Init();
        
        profiler.BeginFrame();
        profiler.EndFrame();
        profiler.RecordAllocation(1024);
        
        profiler.Reset();
        
        const auto& frame = profiler.GetCurrentFrame();
        const auto& mem = profiler.GetMemoryProfile();
        
        REQUIRE(frame.frameNumber == 0);
        REQUIRE(frame.frameTimeMs == 0.0);
        REQUIRE(frame.scopes.empty());
        REQUIRE(mem.totalAllocated == 0);
        REQUIRE(mem.currentUsage == 0);
        
        profiler.Shutdown();
    }
    
    SECTION("Frame Profile String") {
        auto& profiler = vyra::profiler::Profiler::GetInstance();
        profiler.Init();
        
        profiler.BeginFrame();
        profiler.RegisterScope("Scope1", vyra::profiler::ProfileCategory::CPU);
        profiler.RecordScope("Scope1", 5000);
        profiler.EndFrame();
        
        std::string profileString = profiler.GetFrameProfileString();
        REQUIRE(!profileString.empty());
        REQUIRE(profileString.find("Scope1") != std::string::npos);
        
        profiler.Shutdown();
    }
}
