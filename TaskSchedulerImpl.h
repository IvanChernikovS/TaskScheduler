//
// Created by ichernikov on 20.11.23.
//

#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <unordered_set>

#include "ITaskScheduler.h"
#include "Task.h"
#include "ThreadPool.h"

namespace {
inline std::chrono::steady_clock::time_point GetTimePointInTheFuture(int delayInMs) {
  return std::chrono::steady_clock::now() + std::chrono::milliseconds(delayInMs);
}
} // namespace

class TaskSchedulerImpl : public ITaskScheduler {
 private:
  int taskIdCounter;
  std::chrono::steady_clock::time_point wakeUpTime;
  std::multimap<std::chrono::steady_clock::time_point, std::shared_ptr<Task>> taskQueue;
  std::map<int, std::function<void()>> callbacks;
  std::unordered_set<int> taskIds;
  std::condition_variable cv;
  std::atomic_bool isRunning;
  std::mutex mutex;
  ThreadPool pool;

 public:
  explicit TaskSchedulerImpl()
      : isRunning(false), taskIdCounter(0), wakeUpTime(GetTimePointInTheFuture(300)), pool(10) {
    LOG(INFO) << "TaskScheduler::TaskScheduler(): TaskScheduler was created";
  }

  ~TaskSchedulerImpl() noexcept override {
    LOG(INFO) << "TaskScheduler::~TaskScheduler(): Destroying";
    isRunning.store(false);
    cv.notify_one();
  }

  TaskSchedulerImpl(const TaskSchedulerImpl&) = delete;
  TaskSchedulerImpl(TaskSchedulerImpl&&) = delete;

  TaskSchedulerImpl& operator=(const TaskSchedulerImpl&) = delete;
  TaskSchedulerImpl& operator=(TaskSchedulerImpl&&) = delete;

  int Schedule(std::function<void()>&& task, int delay, std::function<void()>&& callback) override {
    std::lock_guard<std::mutex> lock(mutex);

    int id = ++taskIdCounter;
    auto newTask = std::make_unique<Task>(id, std::move(task), std::move(callback), delay);
    const auto taskStartTime = newTask->startTime;

    taskQueue.emplace(taskStartTime, std::move(newTask));
    taskIds.emplace(id);

    UpdateWakeUpTime();

    LOG(INFO) << "TaskScheduler::Schedule(): Task with ID: " << id << " and start time in: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(taskStartTime - std::chrono::steady_clock::now())
                     .count()
              << " ms";

    cv.notify_one();

    return id;
  }

  void Cancel(int taskId) override {
    std::lock_guard<std::mutex> lock(mutex);

    if (taskIds.find(taskId) != taskIds.end()) {
      const auto startTime = std::find_if(taskQueue.cbegin(), taskQueue.cend(), [taskId](const auto& task) {
                               return task.second->taskId == taskId;
                             })->first;

      taskQueue.extract(startTime);
      taskIds.erase(taskId);

      LOG(INFO) << "TaskScheduler::Cancel(): Task with ID: " << taskId << " canceled";
    }
  }

  std::vector<int> GetIncompleteTaskIds() override {
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<int> incompleteTaskIds;
    incompleteTaskIds.reserve(taskIds.size());

    for (const auto& id : taskIds) {
      incompleteTaskIds.emplace_back(id);
    }

    return incompleteTaskIds;
  }

  std::vector<int> GetIncompleteCallbacksIds() override {
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<int> callbackIds;
    callbackIds.reserve(callbacks.size());

    for (const auto& pair : callbacks) {
      callbackIds.emplace_back(pair.first);
    }

    return callbackIds;
  }

  long GetEstimatedStartTime(int taskId) override {
    std::lock_guard<std::mutex> lock(mutex);
    LOG(INFO) << "TaskScheduler::GetEstimatedStartTime(): Estimating start time for task ID: " << taskId;

    return (std::find_if(taskQueue.cbegin(), taskQueue.cend(),
                         [taskId](const auto& task) { return task.second->taskId == taskId; })
                ->first -
            std::chrono::steady_clock::now())
        .count();
  }

  void Start() override {
    if (isRunning.load()) {
      LOG(ERROR) << "TaskScheduler::Start(): Task scheduler is running already";
      return;
    }

    isRunning.store(true);
    LOG(INFO) << "TaskScheduler::Start(): Starting TaskScheduler";

    while (isRunning.load()) {
      std::unique_lock<std::mutex> lock(mutex);

      cv.wait_until(lock, wakeUpTime, [self = this] { return !self->isRunning.load(); });

      if (!isRunning.load()) {
        LOG(INFO) << "TaskScheduler::Start(): taskScheduler stopped";
        return;
      }

      if (taskQueue.empty()) {
        LOG(DEBUG) << "TaskScheduler::Start(): taskQueue is empty";
        UpdateWakeUpTime();
        continue;
      }

      if (wakeUpTime > std::chrono::steady_clock::now()) {
        LOG_IF(!taskQueue.empty(), DEBUG) << "wakeUpTime > now() ";
        continue;
      }

      auto task = taskQueue.cbegin()->second;
      auto id = task->taskId;

      callbacks[id] = task->callback;
      pool.Enqueue(std::move(task));

      taskQueue.extract(taskQueue.cbegin()->first);
      taskIds.erase(id);

      ExecuteCallbacks();

      UpdateWakeUpTime();
    }
  }

  void Stop() override {
    if (!isRunning.load()) {
      LOG(ERROR) << "TaskScheduler::Stop(): TaskScheduler has been stopped already";
    }

    ExecuteCallbacks();

    isRunning.store(false);
    LOG(INFO) << "TaskScheduler::Stop(): Stopping TaskScheduler";

    cv.notify_one();
  }

 private:
  void UpdateWakeUpTime() {
    wakeUpTime = taskQueue.empty() ? GetTimePointInTheFuture(100) : taskQueue.cbegin()->first;
  }

  void ExecuteCallbacks() {
    const auto ids = pool.GetCompletedTaskIds();

    for (const auto& id : ids) {
      callbacks.at(id)();
      callbacks.extract(id);
    }
  }
};
