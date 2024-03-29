//
// Created by ichernikov on 20.11.23.
//

#pragma once

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <unordered_set>

#include "ITaskScheduler.h"

class ThreadPool;
class Task;

class TaskSchedulerImpl : public ITaskScheduler {
 private:
  int taskIdCounter;
  std::chrono::steady_clock::time_point wakeUpTime;
  std::multimap<std::chrono::steady_clock::time_point, std::shared_ptr<Task>> taskQueue;
  std::queue<std::function<void()>> callbacks;
  std::condition_variable cv;
  std::atomic_bool isRunning;
  std::unique_ptr<ThreadPool> pool;
  mutable std::mutex mutex;

 public:
  explicit TaskSchedulerImpl();
  ~TaskSchedulerImpl() noexcept override;

  TaskSchedulerImpl(const TaskSchedulerImpl&) = delete;
  TaskSchedulerImpl(TaskSchedulerImpl&&) noexcept = delete;

  TaskSchedulerImpl& operator=(const TaskSchedulerImpl&) = delete;
  TaskSchedulerImpl& operator=(TaskSchedulerImpl&&) noexcept = delete;

  int Schedule(std::function<void()>&& task, int delay, std::function<void()>&& callback) override;

  void Start() override;
  void Stop() override;
  void CancelTask(int taskId) override;

  void UpdateCallbacks(std::function<void()>&& callback) override;

  std::vector<int> GetIncompleteTaskIds() const override;
  long GetEstimatedStartTime(int taskId) const override;

 private:
  void UpdateWakeUpTime();
  void ExecuteCallbacks();
};
