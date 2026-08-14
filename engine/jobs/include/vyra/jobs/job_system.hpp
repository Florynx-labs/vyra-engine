#pragma once

#include "vyra/core/base.hpp"
#include <functional>
#include <atomic>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>
#include <unordered_map>

namespace vyra {
namespace jobs {

/**
 * @brief Job priority for scheduling
 */
enum class JobPriority : uint8_t {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

/**
 * @brief Job status tracking
 */
enum class JobStatus : uint8_t {
    Pending,
    Running,
    Completed,
    Failed
};

/**
 * @brief Job handle for tracking job execution
 */
class VYRA_API JobHandle {
public:
    JobHandle();
    explicit JobHandle(uint64_t id);
    
    uint64_t GetId() const { return m_id; }
    bool IsValid() const { return m_id != 0; }
    
    bool operator==(const JobHandle& other) const { return m_id == other.m_id; }
    bool operator!=(const JobHandle& other) const { return m_id != other.m_id; }
    
private:
    uint64_t m_id;
    static std::atomic<uint64_t> s_nextId;
};

/**
 * @brief Job function signature
 */
using JobFunction = std::function<void()>;

/**
 * @brief Internal job representation
 */
struct Job {
    JobHandle handle;
    JobFunction function;
    JobPriority priority;
    std::atomic<JobStatus> status;
    std::promise<void> completionPromise;
    
    Job(JobHandle h, JobFunction func, JobPriority prio)
        : handle(h), function(func), priority(prio), status(JobStatus::Pending) {}
};

/**
 * @brief Thread-safe job queue
 */
class VYRA_API JobQueue {
public:
    JobQueue();
    ~JobQueue();
    
    void Push(std::shared_ptr<Job> job);
    std::shared_ptr<Job> Pop();
    bool Empty() const;
    size_t Size() const;
    void Stop();
    bool ShouldStop() const;
    
private:
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    std::queue<std::shared_ptr<Job>> m_queue;
    std::atomic<bool> m_shouldStop{false};
};

/**
 * @brief Worker thread for job execution
 */
class VYRA_API Worker {
public:
    Worker(uint32_t id, JobQueue& queue);
    ~Worker();
    
    void Start();
    void Stop();
    void Join();
    
    uint32_t GetId() const { return m_id; }
    bool IsRunning() const { return m_running; }
    
private:
    void WorkerLoop();
    
    uint32_t m_id;
    JobQueue& m_queue;
    std::thread m_thread;
    std::atomic<bool> m_running;
    std::atomic<bool> m_shouldStop;
};

/**
 * @brief Job scheduler for managing job execution
 */
class VYRA_API JobScheduler {
public:
    static JobScheduler& GetInstance();
    
    void Init(uint32_t workerCount = 0);
    void Shutdown();
    
    /**
     * @brief Schedule a job for execution
     */
    JobHandle Schedule(JobFunction function, JobPriority priority = JobPriority::Normal);
    
    /**
     * @brief Wait for a job to complete
     */
    void Wait(JobHandle handle);
    
    /**
     * @brief Check if a job is complete
     */
    bool IsComplete(JobHandle handle);
    
    /**
     * @brief Get job status
     */
    JobStatus GetStatus(JobHandle handle);
    
    /**
     * @brief Wait for all jobs to complete
     */
    void WaitForAll();
    
    /**
     * @brief Get number of active workers
     */
    uint32_t GetWorkerCount() const { return m_workers.size(); }
    
    /**
     * @brief Get number of pending jobs
     */
    size_t GetPendingJobCount() const;
    
private:
    JobScheduler();
    ~JobScheduler();
    
    std::shared_ptr<Job> FindJob(JobHandle handle);
    
    std::vector<std::unique_ptr<Worker>> m_workers;
    std::unique_ptr<JobQueue> m_queue;
    std::unordered_map<uint64_t, std::shared_ptr<Job>> m_activeJobs;
    std::mutex m_jobsMutex;
    
    std::atomic<bool> m_initialized;
    uint32_t m_workerCount;
};

/**
 * @brief Convenience macro for scheduling jobs
 */
#define VYRA_SCHEDULE_JOB(function) \
    vyra::jobs::JobScheduler::GetInstance().Schedule(function)

#define VYRA_SCHEDULE_JOB_PRIORITY(function, priority) \
    vyra::jobs::JobScheduler::GetInstance().Schedule(function, priority)

} // namespace jobs
} // namespace vyra
