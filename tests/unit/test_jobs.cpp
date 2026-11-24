#include <catch2/catch_test_macros.hpp>
#include <vyra/jobs/job_system.hpp>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <vector>

TEST_CASE("Job handles are unique and invalid by default", "[jobs][unit]") {
    vyra::jobs::JobHandle invalid;
    REQUIRE_FALSE(invalid.IsValid());
    vyra::jobs::JobHandle explicitHandle(9000);
    auto generated = vyra::jobs::JobScheduler::GetInstance();
    (void)generated;
    REQUIRE(explicitHandle.IsValid());
    REQUIRE(explicitHandle.GetId() == 9000);
}

TEST_CASE("Job scheduler executes all queued work and waits safely", "[jobs][unit]") {
    auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
    scheduler.Init(3);

    std::atomic<int> completed{0};
    std::vector<vyra::jobs::JobHandle> handles;
    for (int index = 0; index < 64; ++index) {
        handles.push_back(scheduler.Schedule([&completed] {
            completed.fetch_add(1, std::memory_order_relaxed);
        }));
        REQUIRE(handles.back().IsValid());
    }

    scheduler.WaitForAll();
    REQUIRE(completed.load() == 64);
    REQUIRE(scheduler.GetPendingJobCount() == 0);
    for (const auto handle : handles) {
        REQUIRE(scheduler.IsComplete(handle));
        REQUIRE(scheduler.GetStatus(handle) == vyra::jobs::JobStatus::Completed);
        scheduler.Wait(handle); // shared completion state supports repeated waits
    }
    scheduler.Shutdown();
}

TEST_CASE("Job scheduler reports failures without hanging waiters", "[jobs][unit]") {
    auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
    scheduler.Init(2);
    const auto handle = scheduler.Schedule([] {
        throw std::runtime_error("expected job failure");
    });
    REQUIRE(handle.IsValid());
    scheduler.Wait(handle);
    REQUIRE(scheduler.GetStatus(handle) == vyra::jobs::JobStatus::Failed);
    REQUIRE(scheduler.IsComplete(handle));
    scheduler.Shutdown();
}

TEST_CASE("Job scheduler drains queued work during shutdown", "[jobs][unit]") {
    auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
    scheduler.Init(1);
    std::atomic<int> completed{0};
    for (int index = 0; index < 40; ++index) {
        scheduler.Schedule([&completed] {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            completed.fetch_add(1, std::memory_order_relaxed);
        });
    }
    scheduler.Shutdown();
    REQUIRE(completed.load() == 40);
    REQUIRE(scheduler.GetWorkerCount() == 0);
    REQUIRE_FALSE(scheduler.Schedule([] {}).IsValid());
}

TEST_CASE("Job scheduler supports repeated initialization cycles", "[jobs][unit]") {
    auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
    for (int cycle = 0; cycle < 3; ++cycle) {
        scheduler.Init(2);
        std::atomic<bool> ran{false};
        const auto handle = scheduler.Schedule([&ran] { ran = true; });
        scheduler.Wait(handle);
        REQUIRE(ran.load());
        scheduler.Shutdown();
    }
}
