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

class ThreadPool {
 private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::vector<int> completedTaskIds;
  std::mutex mutex;
  std::condition_variable cv;
  std::atomic_bool stop;

 public:
  explicit ThreadPool(int);
  ~ThreadPool() noexcept;

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;

  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  void Enqueue(const std::shared_ptr<Task>&& task);

  std::vector<int> GetCompletedTaskIds();
};