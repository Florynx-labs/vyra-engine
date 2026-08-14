#include "vyra/jobs/job_system.hpp"
#include "vyra/core/log.hpp"
#include <algorithm>
#include <chrono>

namespace vyra {
namespace jobs {

// JobHandle implementation
std::atomic<uint64_t> JobHandle::s_nextId{1};

JobHandle::JobHandle() : m_id(0) {}

JobHandle::JobHandle(uint64_t id) : m_id(id) {
    if (id == 0) {
        m_id = s_nextId.fetch_add(1);
    }
}

// JobQueue implementation
JobQueue::JobQueue() {}

JobQueue::~JobQueue() {
    std::lock_guard<std::mutex> lock(m_mutex);
    while (!m_queue.empty()) {
        m_queue.pop();
    }
}

void JobQueue::Push(std::shared_ptr<Job> job) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(job);
    m_condition.notify_one();
}

std::shared_ptr<Job> JobQueue::Pop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait_for(lock, std::chrono::milliseconds(100), [this] { 
        return !m_queue.empty() || m_shouldStop; 
    });
    
    if (m_shouldStop || m_queue.empty()) {
        return nullptr;
    }
    
    auto job = m_queue.front();
    m_queue.pop();
    return job;
}

bool JobQueue::ShouldStop() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_shouldStop;
}

void JobQueue::Stop() {
    m_shouldStop = true;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_condition.notify_all();
}

bool JobQueue::Empty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

size_t JobQueue::Size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

// Worker implementation
Worker::Worker(uint32_t id, JobQueue& queue)
    : m_id(id), m_queue(queue), m_running(false), m_shouldStop(false) {}

Worker::~Worker() {
    Stop();
    Join();
}

void Worker::Start() {
    if (m_running) return;
    
    m_running = true;
    m_shouldStop = false;
    m_thread = std::thread(&Worker::WorkerLoop, this);
    
    VYRA_LOG_CHANNEL(LogChannel::Core, info, "Worker {0} started", m_id);
}

void Worker::Stop() {
    m_shouldStop = true;
}

void Worker::Join() {
    if (m_thread.joinable()) {
        m_thread.join();
        m_running = false;
    }
}

void Worker::WorkerLoop() {
    while (!m_shouldStop && !m_queue.ShouldStop()) {
        try {
            auto job = m_queue.Pop();
            
            if (m_queue.ShouldStop()) {
                break;
            }
            
            if (job) {
                job->status = JobStatus::Running;
                
                try {
                    job->function();
                    job->status = JobStatus::Completed;
                    job->completionPromise.set_value();
                } catch (const std::exception& e) {
                    job->status = JobStatus::Failed;
                    job->completionPromise.set_exception(std::current_exception());
                    VYRA_LOG_CHANNEL(LogChannel::Core, error, "Job {0} failed: {1}", 
                                     job->handle.GetId(), e.what());
                }
            }
        } catch (const std::exception& e) {
            VYRA_LOG_CHANNEL(LogChannel::Core, error, "Worker {0} error: {1}", m_id, e.what());
        }
    }
}

// JobScheduler implementation
JobScheduler::JobScheduler()
    : m_initialized(false), m_workerCount(0) {}

JobScheduler::~JobScheduler() {
    Shutdown();
}

JobScheduler& JobScheduler::GetInstance() {
    static JobScheduler instance;
    return instance;
}

void JobScheduler::Init(uint32_t workerCount) {
    if (m_initialized) {
        VYRA_LOG_CHANNEL(LogChannel::Core, warn, "JobScheduler already initialized");
        return;
    }
    
    m_queue = std::make_unique<JobQueue>();
    
    // Determine worker count (hardware concurrency or specified)
    if (workerCount == 0) {
        workerCount = std::thread::hardware_concurrency();
        if (workerCount == 0) workerCount = 4; // Fallback
    }
    
    m_workerCount = workerCount;
    
    // Create workers
    for (uint32_t i = 0; i < m_workerCount; ++i) {
        m_workers.push_back(std::make_unique<Worker>(i, *m_queue));
        m_workers.back()->Start();
    }
    
    m_initialized = true;
    VYRA_LOG_CHANNEL(LogChannel::Core, info, "JobScheduler initialized with {0} workers", m_workerCount);
}

void JobScheduler::Shutdown() {
    if (!m_initialized) return;
    
    VYRA_LOG_CHANNEL(LogChannel::Core, info, "Shutting down JobScheduler");
    
    // Stop queue to wake up workers
    if (m_queue) {
        m_queue->Stop();
    }
    
    // Stop all workers
    for (auto& worker : m_workers) {
        worker->Stop();
    }
    
    // Join all workers
    for (auto& worker : m_workers) {
        worker->Join();
    }
    
    m_workers.clear();
    m_queue.reset();
    
    {
        std::lock_guard<std::mutex> lock(m_jobsMutex);
        m_activeJobs.clear();
    }
    
    m_initialized = false;
    VYRA_LOG_CHANNEL(LogChannel::Core, info, "JobScheduler shut down");
}

JobHandle JobScheduler::Schedule(JobFunction function, JobPriority priority) {
    if (!m_initialized) {
        VYRA_LOG_CHANNEL(LogChannel::Core, error, "Cannot schedule job - JobScheduler not initialized");
        return JobHandle();
    }
    
    JobHandle handle;
    auto job = std::make_shared<Job>(handle, function, priority);
    
    {
        std::lock_guard<std::mutex> lock(m_jobsMutex);
        m_activeJobs[handle.GetId()] = job;
    }
    
    m_queue->Push(job);
    
    return handle;
}

void JobScheduler::Wait(JobHandle handle) {
    auto job = FindJob(handle);
    if (job) {
        job->completionPromise.get_future().wait();
    }
}

bool JobScheduler::IsComplete(JobHandle handle) {
    auto job = FindJob(handle);
    if (!job) return false;
    
    return job->status == JobStatus::Completed;
}

JobStatus JobScheduler::GetStatus(JobHandle handle) {
    auto job = FindJob(handle);
    if (!job) return JobStatus::Failed;
    
    return job->status;
}

void JobScheduler::WaitForAll() {
    // Wait for all jobs to complete
    while (true) {
        {
            std::lock_guard<std::mutex> lock(m_jobsMutex);
            bool allComplete = true;
            
            for (const auto& [id, job] : m_activeJobs) {
                if (job->status != JobStatus::Completed && job->status != JobStatus::Failed) {
                    allComplete = false;
                    break;
                }
            }
            
            if (allComplete && m_queue->Empty()) {
                break;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

size_t JobScheduler::GetPendingJobCount() const {
    return m_queue->Size();
}

std::shared_ptr<Job> JobScheduler::FindJob(JobHandle handle) {
    std::lock_guard<std::mutex> lock(m_jobsMutex);
    auto it = m_activeJobs.find(handle.GetId());
    if (it != m_activeJobs.end()) {
        return it->second;
    }
    return nullptr;
}

} // namespace jobs
} // namespace vyra
