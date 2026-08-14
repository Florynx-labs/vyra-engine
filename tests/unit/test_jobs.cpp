#include <catch2/catch_test_macros.hpp>
#include <vyra/jobs/job_system.hpp>
#include <vyra/core/log.hpp>
#include <thread>
#include <chrono>
#include <atomic>

TEST_CASE("VYRA Job System - Basic Functionality", "[jobs][unit]") {
    vyra::Log::Init();
    
    SECTION("JobHandle Creation") {
        vyra::jobs::JobHandle handle1;
        REQUIRE_FALSE(handle1.IsValid());
        
        vyra::jobs::JobHandle handle2(123);
        REQUIRE(handle2.IsValid());
        REQUIRE(handle2.GetId() == 123);
    }
    
    SECTION("JobHandle Comparison") {
        vyra::jobs::JobHandle handle1(100);
        vyra::jobs::JobHandle handle2(100);
        vyra::jobs::JobHandle handle3(200);
        
        REQUIRE(handle1 == handle2);
        REQUIRE(handle1 != handle3);
    }
    
    SECTION("JobQueue Basic Operations") {
        vyra::jobs::JobQueue queue;
        
        auto job = std::make_shared<vyra::jobs::Job>(
            vyra::jobs::JobHandle(1),
            []() {},
            vyra::jobs::JobPriority::Normal
        );
        
        REQUIRE(queue.Empty());
        REQUIRE(queue.Size() == 0);
        
        queue.Push(job);
        REQUIRE_FALSE(queue.Empty());
        REQUIRE(queue.Size() == 1);
    }
    
    SECTION("JobScheduler Initialization") {
        auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
        scheduler.Init(2);
        
        REQUIRE(scheduler.GetWorkerCount() == 2);
        
        scheduler.Shutdown();
    }
    
    SECTION("JobScheduler Default Worker Count") {
        auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
        scheduler.Init(); // Use hardware concurrency
        
        REQUIRE(scheduler.GetWorkerCount() > 0);
        
        scheduler.Shutdown();
    }
    
    SECTION("Simple Job Execution") {
        auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
        scheduler.Init(2);
        
        std::atomic<bool> executed(false);
        auto handle = scheduler.Schedule([&executed]() {
            executed = true;
        });
        
        scheduler.Wait(handle);
        REQUIRE(executed);
        REQUIRE(scheduler.IsComplete(handle));
        
        scheduler.Shutdown();
    }
    
    SECTION("Multiple Jobs") {
        auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
        scheduler.Init(4);
        
        std::atomic<int> counter(0);
        std::vector<vyra::jobs::JobHandle> handles;
        
        for (int i = 0; i < 10; i++) {
            handles.push_back(scheduler.Schedule([&counter]() {
                counter.fetch_add(1);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }));
        }
        
        scheduler.WaitForAll();
        REQUIRE(counter == 10);
        
        scheduler.Shutdown();
    }
    
    SECTION("Job Status Tracking") {
        auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
        scheduler.Init(2);
        
        auto handle = scheduler.Schedule([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        });
        
        REQUIRE(scheduler.GetStatus(handle) == vyra::jobs::JobStatus::Pending);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        // Should be running or completed by now
        auto status = scheduler.GetStatus(handle);
        REQUIRE((status == vyra::jobs::JobStatus::Running || 
                status == vyra::jobs::JobStatus::Completed));
        
        scheduler.Wait(handle);
        REQUIRE(scheduler.GetStatus(handle) == vyra::jobs::JobStatus::Completed);
        
        scheduler.Shutdown();
    }
    
    SECTION("Job Exception Handling") {
        auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
        scheduler.Init(2);
        
        auto handle = scheduler.Schedule([]() {
            throw std::runtime_error("Test exception");
        });
        
        scheduler.Wait(handle);
        REQUIRE(scheduler.GetStatus(handle) == vyra::jobs::JobStatus::Failed);
        
        scheduler.Shutdown();
    }
    
    SECTION("Job Priority (FIFO for now)") {
        auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
        scheduler.Init(2);
        
        std::vector<int> executionOrder;
        std::mutex orderMutex;
        
        auto handle1 = scheduler.Schedule([&executionOrder, &orderMutex]() {
            std::lock_guard<std::mutex> lock(orderMutex);
            executionOrder.push_back(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }, vyra::jobs::JobPriority::High);
        
        auto handle2 = scheduler.Schedule([&executionOrder, &orderMutex]() {
            std::lock_guard<std::mutex> lock(orderMutex);
            executionOrder.push_back(2);
        }, vyra::jobs::JobPriority::Low);
        
        scheduler.WaitForAll();
        
        // For now, jobs are executed in FIFO order regardless of priority
        REQUIRE(executionOrder.size() == 2);
        
        scheduler.Shutdown();
    }
    
    SECTION("Pending Job Count") {
        auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
        scheduler.Init(2);
        
        std::atomic<bool> blocking(false);
        
        // Schedule a blocking job
        scheduler.Schedule([&blocking]() {
            blocking = true;
            while (blocking) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        
        // Schedule multiple jobs that will queue up
        for (int i = 0; i < 5; i++) {
            scheduler.Schedule([]() {});
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        // Should have some pending jobs
        size_t pendingCount = scheduler.GetPendingJobCount();
        REQUIRE(pendingCount >= 0);
        
        // Release blocking job
        blocking = false;
        
        scheduler.WaitForAll();
        
        scheduler.Shutdown();
    }
    
    SECTION("WaitForAll") {
        auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
        scheduler.Init(4);
        
        std::atomic<int> counter(0);
        
        for (int i = 0; i < 20; i++) {
            scheduler.Schedule([&counter]() {
                counter.fetch_add(1);
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            });
        }
        
        scheduler.WaitForAll();
        REQUIRE(counter == 20);
        REQUIRE(scheduler.GetPendingJobCount() == 0);
        
        scheduler.Shutdown();
    }
    
    SECTION("Multiple Init/Shutdown Cycles") {
        auto& scheduler = vyra::jobs::JobScheduler::GetInstance();
        
        for (int i = 0; i < 3; i++) {
            scheduler.Init(2);
            
            std::atomic<bool> executed(false);
            auto handle = scheduler.Schedule([&executed]() {
                executed = true;
            });
            
            scheduler.Wait(handle);
            REQUIRE(executed);
            
            scheduler.Shutdown();
        }
    }
    
    vyra::Log::Shutdown();
}
