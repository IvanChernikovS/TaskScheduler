//
// Created by ichernikov on 20.11.23.
//

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

class Task;
class ITaskScheduler;

class ThreadPool {
 private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::vector<int> completedTaskIds;
  std::mutex mutex;
  std::condition_variable cv;
  std::atomic_bool stop;
  ITaskScheduler* taskScheduler;

 public:
  explicit ThreadPool(int);
  ~ThreadPool() noexcept;

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;

  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  void AttachTaskScheduler(ITaskScheduler* observer);
  void DetachTaskScheduler();
  void NotifyTaskScheduler(std::function<void()> callback);

  void Enqueue(const std::shared_ptr<Task>& task);
};