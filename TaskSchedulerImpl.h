//
// Created by ichernikov on 20.11.23.
//

#pragma once

#include <condition_variable>
#include <map>
#include <mutex>
#include <unordered_set>

#include "ITaskScheduler.h"
#include "ThreadPool.h"

class TaskSchedulerImpl : public ITaskScheduler {
 private:
  int taskIdCounter;
  std::chrono::steady_clock::time_point wakeUpTime;
  std::multimap<std::chrono::steady_clock::time_point, std::shared_ptr<Task>> taskQueue;
  std::map<int, std::function<void()>> callbacks;
  std::unordered_set<int> taskIds;
  std::condition_variable cv;
  std::atomic_bool isRunning;
  ThreadPool pool;
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
  void Cancel(int taskId) override;

  std::vector<int> GetIncompleteTaskIds() const override;
  std::vector<int> GetIncompleteCallbacksIds() const override;
  long GetEstimatedStartTime(int taskId) const override;

 private:
  void UpdateWakeUpTime();
  void ExecuteCallbacks();
};
