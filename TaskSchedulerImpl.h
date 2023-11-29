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

class TaskSchedulerImpl : public ITaskScheduler {
 private:
  int taskIdCounter;
  std::map<int, std::vector<std::shared_ptr<Task>>> taskQueue;
  std::unordered_set<int> taskIds;
  std::condition_variable cv;
  std::mutex mutex;
  ThreadPool pool;
  std::atomic_bool isRunning;

 public:
  explicit TaskSchedulerImpl() : taskIdCounter(0), pool(10), isRunning(false) {
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

  int Schedule(std::function<void()>&& task, int delay, int priority,
               std::function<void()>&& callback) override {
    std::lock_guard<std::mutex> lock(mutex);

    int id = ++taskIdCounter;
    auto newTask =
        std::make_unique<Task>(id, std::move(task), std::move(callback), priority, delay);
    taskQueue[priority].emplace_back(std::move(newTask));
    taskIds.emplace(id);

    LOG(INFO) << "TaskScheduler::Schedule(): Task with ID: " << id << " and priority: " << priority
              << " scheduled";

    cv.notify_one();

    return id;
  }

  int ScheduleCompletingTask() override {
    // std::lock_guard<std::mutex> lock(mutex);
    auto task = [] { LOG(INFO) << "Running completing task"; };
    auto callback = [self = this] {
      LOG(INFO) << "The callback of completing task";

      if (!self->taskQueue.empty()) {
        LOG(INFO) << "Task queue is not empty, there are still {"
                  << self->GetIncompleteTaskIds().size()
                  << "} incomplete tasks. Scheduling completing task again";
        self->ScheduleCompletingTask();
        return;
      }

      self->isRunning.store(false);
      self->cv.notify_one();
    };

    return Schedule(task, 1000, 10, callback);
  }

  void Cancel(int taskId) override {
    std::lock_guard<std::mutex> lock(mutex);

    if (taskIds.find(taskId) != taskIds.end()) {
      // TODO: erase from taskQueue
      taskIds.erase(taskId);

      LOG(INFO) << "TaskScheduler::Cancel(): Task with ID: " << taskId << " canceled";
    }
  }

  std::vector<int> GetIncompleteTaskIds() override {
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<int> incompleteTaskIds;
    for (const auto& id : taskIds) {
      incompleteTaskIds.push_back(id);
    }

    LOG(INFO) << "TaskScheduler::GetIncompleteTasks(): There are " << incompleteTaskIds.size()
              << " incomplete tasks";

    return incompleteTaskIds;
  }

  int GetEstimatedStartTime(int taskId) override {
    LOG(INFO) << "TaskScheduler::getEstimatedStartTime(): Estimating start time for task ID: "
              << taskId;
    return -1;
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
      auto pred = [self = this] { return !self->isRunning.load() || !self->taskQueue.empty(); };
      if (!cv.wait_for(lock, std::chrono::minutes(1), pred)) {
        LOG(INFO) << "Waiting for tasks timed out";
        break;
      }

      for (auto& tasks : taskQueue) {
        for (auto& task : tasks.second) {
          if (task && task->startTime <= std::chrono::system_clock::now()) {
            auto id = task->taskId;
            pool.Enqueue(std::move(task));
            taskIds.erase(id);
            task = nullptr;
          }
        }
      }

      ClearTaskQueue();
    }
  }

  void Stop() override {
    if (!isRunning.load()) {
      LOG(ERROR) << "TaskScheduler::Stop(): TaskScheduler has been stopped already";
    }

    isRunning.store(false);
    LOG(INFO) << "TaskScheduler::Stop(): Stopping TaskScheduler";

    cv.notify_one();
  }

 private:
  void ClearTaskQueue() {
    if (taskQueue.empty()) {
      return;
    }

    size_t counter = 0;
    for (auto it = taskQueue.begin(); it != taskQueue.end();) {
      if (it->second.empty() || std::all_of(it->second.cbegin(), it->second.cend(),
                                            [](const auto& task) { return !task; })) {
        counter += it->second.size();
        it = taskQueue.erase(it);
      } else {
        ++it;
      }
    }

    LOG_IF(counter > 0, DEBUG) << "Cleared "<< counter << " tasks";
  }
};
