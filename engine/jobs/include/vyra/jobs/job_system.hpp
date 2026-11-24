#pragma once

#include "vyra/core/base.hpp"
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

namespace vyra::jobs {

enum class JobPriority : uint8_t { Low = 0, Normal = 1, High = 2, Critical = 3 };
enum class JobStatus : uint8_t { Pending, Running, Completed, Failed };

class VYRA_API JobHandle {
public:
    JobHandle() = default;
    explicit JobHandle(uint64_t id);
    uint64_t GetId() const noexcept { return m_id; }
    bool IsValid() const noexcept { return m_id != 0; }
    bool operator==(const JobHandle& other) const noexcept { return m_id == other.m_id; }
    bool operator!=(const JobHandle& other) const noexcept { return !(*this == other); }
private:
    friend class JobScheduler;
    static JobHandle Generate();
    uint64_t m_id{0};
    static std::atomic<uint64_t> s_nextId;
};

using JobFunction = std::function<void()>;

struct Job {
    JobHandle handle;
    JobFunction function;
    JobPriority priority;
    std::atomic<JobStatus> status{JobStatus::Pending};
    std::promise<void> completionPromise;
    std::shared_future<void> completion;

    Job(JobHandle jobHandle, JobFunction jobFunction, JobPriority jobPriority)
        : handle(jobHandle), function(std::move(jobFunction)), priority(jobPriority),
          completion(completionPromise.get_future().share()) {}
};

class VYRA_API JobQueue {
public:
    bool Push(std::shared_ptr<Job> job);
    std::shared_ptr<Job> Pop();
    bool Empty() const;
    size_t Size() const;
    void Stop();
    bool ShouldStop() const;
private:
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    std::queue<std::shared_ptr<Job>> m_queue;
    bool m_shouldStop{false};
};

class VYRA_API Worker {
public:
    Worker(uint32_t id, JobQueue& queue, std::atomic<size_t>& outstanding,
           std::condition_variable& completionCondition, std::mutex& completionMutex);
    ~Worker();
    void Start();
    void Stop();
    void Join();
    uint32_t GetId() const noexcept { return m_id; }
    bool IsRunning() const noexcept { return m_running.load(); }
private:
    void WorkerLoop();
    uint32_t m_id;
    JobQueue& m_queue;
    std::atomic<size_t>& m_outstanding;
    std::condition_variable& m_completionCondition;
    std::mutex& m_completionMutex;
    std::thread m_thread;
    std::atomic<bool> m_running{false};
};

class VYRA_API JobScheduler {
public:
    static JobScheduler& GetInstance();
    void Init(uint32_t workerCount = 0);
    void Shutdown();
    JobHandle Schedule(JobFunction function, JobPriority priority = JobPriority::Normal);
    void Wait(JobHandle handle);
    bool IsComplete(JobHandle handle);
    JobStatus GetStatus(JobHandle handle);
    void WaitForAll();
    uint32_t GetWorkerCount() const noexcept { return m_workerCount.load(); }
    size_t GetPendingJobCount() const;

private:
    JobScheduler() = default;
    ~JobScheduler();
    std::shared_ptr<Job> FindJob(JobHandle handle);
    mutable std::mutex m_lifecycleMutex;
    std::vector<std::unique_ptr<Worker>> m_workers;
    std::unique_ptr<JobQueue> m_queue;
    std::unordered_map<uint64_t, std::shared_ptr<Job>> m_activeJobs;
    std::mutex m_jobsMutex;
    std::atomic<size_t> m_outstanding{0};
    std::condition_variable m_completionCondition;
    std::mutex m_completionMutex;
    std::atomic<bool> m_initialized{false};
    std::atomic<bool> m_accepting{false};
    std::atomic<uint32_t> m_workerCount{0};
};

#define VYRA_SCHEDULE_JOB(function) \
    ::vyra::jobs::JobScheduler::GetInstance().Schedule(function)
#define VYRA_SCHEDULE_JOB_PRIORITY(function, priority) \
    ::vyra::jobs::JobScheduler::GetInstance().Schedule(function, priority)

} // namespace vyra::jobs
