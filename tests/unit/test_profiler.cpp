#include <catch2/catch_test_macros.hpp>
#include <vyra/profiler/profiler.hpp>
#include <atomic>
#include <thread>
#include <vector>

TEST_CASE("Profiler records frame and scope statistics", "[profiler][unit]") {
    auto& profiler = vyra::profiler::Profiler::GetInstance();
    profiler.Init();
    profiler.BeginFrame();
    profiler.RegisterScope("Update", vyra::profiler::ProfileCategory::CPU);
    profiler.RecordScope("Update", 1000);
    profiler.RecordScope("Update", 3000);
    profiler.EndFrame();

    const auto frame = profiler.GetCurrentFrame();
    REQUIRE(frame.frameNumber == 0);
    REQUIRE(frame.frameTimeMs >= 0.0);
    REQUIRE(frame.scopes.at("Update").callCount == 2);
    REQUIRE(frame.scopes.at("Update").totalDurationUs == 4000);
    REQUIRE(frame.scopes.at("Update").averageDurationUs == 2000);
    REQUIRE(frame.scopes.at("Update").minDurationUs == 1000);
    REQUIRE(frame.scopes.at("Update").maxDurationUs == 3000);
    REQUIRE(!profiler.GetFrameProfileString().empty());
    profiler.Shutdown();
}

TEST_CASE("Profiler memory accounting never underflows", "[profiler][memory]") {
    auto& profiler = vyra::profiler::Profiler::GetInstance();
    profiler.Init();
    profiler.Reset();
    profiler.RecordAllocation(1024);
    profiler.RecordAllocation(2048);
    profiler.RecordFree(512);
    auto memory = profiler.GetMemoryProfile();
    REQUIRE(memory.totalAllocated == 3072);
    REQUIRE(memory.totalFreed == 512);
    REQUIRE(memory.currentUsage == 2560);
    REQUIRE(memory.peakUsage == 3072);

    profiler.RecordFree(99999);
    memory = profiler.GetMemoryProfile();
    REQUIRE(memory.currentUsage == 0);
    REQUIRE(memory.totalFreed == 3072);
    profiler.Shutdown();
}

TEST_CASE("Profiler snapshots are safe during concurrent recording", "[profiler][threading]") {
    auto& profiler = vyra::profiler::Profiler::GetInstance();
    profiler.Init();
    profiler.Reset();
    std::vector<std::thread> workers;
    for (int threadIndex = 0; threadIndex < 4; ++threadIndex) {
        workers.emplace_back([&profiler, threadIndex] {
            for (int iteration = 0; iteration < 250; ++iteration) {
                profiler.RecordScope("worker-" + std::to_string(threadIndex), 5);
                profiler.RecordAllocation(8);
                (void)profiler.GetCurrentFrame();
                (void)profiler.GetMemoryProfile();
            }
        });
    }
    for (auto& worker : workers) worker.join();
    REQUIRE(profiler.GetMemoryProfile().currentUsage == 8000);
    profiler.Shutdown();
}
