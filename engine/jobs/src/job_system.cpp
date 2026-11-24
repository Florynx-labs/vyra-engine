#include "vyra/jobs/job_system.hpp"
#include "vyra/core/log.hpp"
#include <algorithm>
#include <exception>
#include <stdexcept>

namespace vyra::jobs {

std::atomic<uint64_t> JobHandle::s_nextId{1};

JobHandle::JobHandle(uint64_t id) : m_id(id) {
    if (id == 0) return;
    uint64_t expected = s_nextId.load(std::memory_order_relaxed);
    while (expected <= id &&
           !s_nextId.compare_exchange_weak(expected, id + 1,
                                           std::memory_order_relaxed)) {}
}

JobHandle JobHandle::Generate() {
    return JobHandle(s_nextId.fetch_add(1, std::memory_order_relaxed));
}

bool JobQueue::Push(std::shared_ptr<Job> job) {
    if (!job) return false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_shouldStop) return false;
        m_queue.push(std::move(job));
    }
    m_condition.notify_one();
    return true;
}

std::shared_ptr<Job> JobQueue::Pop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait(lock, [this] { return m_shouldStop || !m_queue.empty(); });
    // Drain queued work before workers exit during shutdown.
    if (m_queue.empty()) return nullptr;
    auto job = std::move(m_queue.front());
    m_queue.pop();
    return job;
}

bool JobQueue::Empty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

size_t JobQueue::Size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

void JobQueue::Stop() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_shouldStop = true;
    }
    m_condition.notify_all();
}

bool JobQueue::ShouldStop() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_shouldStop;
}

Worker::Worker(uint32_t id, JobQueue& queue, std::atomic<size_t>& outstanding,
               std::condition_variable& completionCondition, std::mutex& completionMutex)
    : m_id(id), m_queue(queue), m_outstanding(outstanding),
      m_completionCondition(completionCondition), m_completionMutex(completionMutex) {}

Worker::~Worker() {
    Stop();
    Join();
}

void Worker::Start() {
    if (m_running.exchange(true)) return;
    m_shouldStop = false;
    m_thread = std::thread(&Worker::WorkerLoop, this);
}

void Worker::Stop() {
    m_shouldStop = true;
}

void Worker::Join() {
    if (m_thread.joinable()) m_thread.join();
    m_running = false;
}

void Worker::WorkerLoop() {
    while (true) {
        auto job = m_queue.Pop();
        if (!job) break;

        job->status = JobStatus::Running;
        try {
            if (!job->function) throw std::invalid_argument("Scheduled job has no function");
            job->function();
            job->status = JobStatus::Completed;
            job->completionPromise.set_value();
        } catch (...) {
            job->status = JobStatus::Failed;
            try { job->completionPromise.set_exception(std::current_exception()); } catch (...) {}
            try {
                throw;
            } catch (const std::exception& error) {
                VYRA_LOG_CHANNEL(LogChannel::Core, error, "Job {0} failed: {1}",
                                 job->handle.GetId(), error.what());
            } catch (...) {
                VYRA_LOG_CHANNEL(LogChannel::Core, error,
                                 "Job {0} failed with a non-standard exception",
                                 job->handle.GetId());
            }
        }

        m_outstanding.fetch_sub(1, std::memory_order_acq_rel);
        m_completionCondition.notify_all();
    }
    m_running = false;
}

JobScheduler& JobScheduler::GetInstance() {
    static JobScheduler instance;
    return instance;
}

JobScheduler::~JobScheduler() {
    Shutdown();
}

void JobScheduler::Init(uint32_t workerCount) {
    std::lock_guard<std::mutex> lifecycleLock(m_lifecycleMutex);
    if (m_initialized.load()) return;

    if (workerCount == 0) {
        workerCount = std::thread::hardware_concurrency();
        if (workerCount == 0) workerCount = 1;
    }

    m_queue = std::make_unique<JobQueue>();
    m_workers.reserve(workerCount);
    m_workerCount = workerCount;
    m_accepting = true;

    try {
        for (uint32_t index = 0; index < workerCount; ++index) {
            auto worker = std::make_unique<Worker>(
                index, *m_queue, m_outstanding, m_completionCondition, m_completionMutex);
            worker->Start();
            m_workers.push_back(std::move(worker));
        }
    } catch (...) {
        m_accepting = false;
        m_queue->Stop();
        for (auto& worker : m_workers) worker->Join();
        m_workers.clear();
        m_queue.reset();
        m_workerCount = 0;
        throw;
    }

    m_initialized = true;
    VYRA_LOG_CHANNEL(LogChannel::Core, info, "JobScheduler initialized with {0} workers",
                     workerCount);
}

void JobScheduler::Shutdown() {
    {
        std::lock_guard<std::mutex> lifecycleLock(m_lifecycleMutex);
        if (!m_initialized.load()) return;
        m_accepting = false;
    }

    // Do not abandon queued work or leave handles permanently Pending.
    WaitForAll();

    std::lock_guard<std::mutex> lifecycleLock(m_lifecycleMutex);
    if (m_queue) m_queue->Stop();
    for (auto& worker : m_workers) worker->Join();
    m_workers.clear();
    m_queue.reset();
    m_workerCount = 0;
    m_initialized = false;
    VYRA_LOG_CHANNEL(LogChannel::Core, info, "JobScheduler shut down");
}

JobHandle JobScheduler::Schedule(JobFunction function, JobPriority priority) {
    std::lock_guard<std::mutex> lifecycleLock(m_lifecycleMutex);
    if (!m_initialized.load() || !m_accepting.load() || !m_queue) {
        VYRA_LOG_CHANNEL(LogChannel::Core, error,
                         "Cannot schedule job: scheduler is not accepting work");
        return {};
    }

    const JobHandle handle = JobHandle::Generate();
    auto job = std::make_shared<Job>(handle, std::move(function), priority);
    {
        std::lock_guard<std::mutex> jobsLock(m_jobsMutex);
        m_activeJobs.emplace(handle.GetId(), job);
    }

    m_outstanding.fetch_add(1, std::memory_order_release);
    if (!m_queue->Push(job)) {
        m_outstanding.fetch_sub(1, std::memory_order_acq_rel);
        std::lock_guard<std::mutex> jobsLock(m_jobsMutex);
        m_activeJobs.erase(handle.GetId());
        return {};
    }
    return handle;
}

std::shared_ptr<Job> JobScheduler::FindJob(JobHandle handle) {
    if (!handle.IsValid()) return nullptr;
    std::lock_guard<std::mutex> lock(m_jobsMutex);
    const auto it = m_activeJobs.find(handle.GetId());
    return it == m_activeJobs.end() ? nullptr : it->second;
}

void JobScheduler::Wait(JobHandle handle) {
    if (auto job = FindJob(handle)) job->completion.wait();
}

bool JobScheduler::IsComplete(JobHandle handle) {
    const auto status = GetStatus(handle);
    return status == JobStatus::Completed || status == JobStatus::Failed;
}

JobStatus JobScheduler::GetStatus(JobHandle handle) {
    const auto job = FindJob(handle);
    return job ? job->status.load() : JobStatus::Failed;
}

void JobScheduler::WaitForAll() {
    std::unique_lock<std::mutex> lock(m_completionMutex);
    m_completionCondition.wait(lock, [this] {
        return m_outstanding.load(std::memory_order_acquire) == 0;
    });
}

size_t JobScheduler::GetPendingJobCount() const {
    std::lock_guard<std::mutex> lifecycleLock(m_lifecycleMutex);
    return m_queue ? m_queue->Size() : 0;
}

} // namespace vyra::jobs
