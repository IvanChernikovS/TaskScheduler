//
// Created by ichernikov on 20.11.23.
//

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <queue>
#include <thread>
#include <vector>

#include "Task.h"

class ThreadPool {
 private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::mutex queue_mutex;
  std::condition_variable cv;
  std::atomic_bool stop;

 public:
  explicit ThreadPool(int);
  ~ThreadPool() noexcept;

  void Enqueue(const std::shared_ptr<Task> task);
};

ThreadPool::ThreadPool(int threads) : stop(false) {
  LOG(INFO) << "ThreadPool::ThreadPool(): Creating ThreadPool with " << threads << " threads";

  for (auto i = 0; i < threads; ++i)
    workers.emplace_back([this, i] {
      for (;;) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(this->queue_mutex);
          this->cv.wait(lock, [this] { return this->stop.load() || !this->tasks.empty(); });
          if (this->stop.load() && this->tasks.empty()) {
            LOG(INFO) << "ThreadPool::ThreadPool(): Worker " << i
                      << " is stopping because ThreadPool is stopped. Exiting thread";
            return;
          }
          task = std::move(this->tasks.front());
          this->tasks.pop();
        }

        LOG(INFO) << "ThreadPool::ThreadPool(): Worker " << i << " is running a task";
        task();
      }
    });
}

ThreadPool::~ThreadPool() noexcept {
  LOG(INFO) << "ThreadPool::~ThreadPool(): Destroying ThreadPool";
  stop.store(true);

  cv.notify_all();

  for (auto& worker : workers) {
    if (worker.joinable()) {
      worker.join();
      LOG(INFO) << "ThreadPool::~ThreadPool(): Joined worker thread with ID: " << worker.get_id();
    }
  }
}

void ThreadPool::Enqueue(const std::shared_ptr<Task> task) {
  {
    LOG(INFO) << "ThreadPool::Enqueue(): Enqueuing task with ID: " << task->taskId;
    std::lock_guard<std::mutex> lock(queue_mutex);
    tasks.emplace([task]() {
      LOG(INFO) << "ThreadPool::Enqueue(): Running task with ID: " << task->taskId
                << " and its callback";
      task->task();
      task->callback();
    });
  }
  cv.notify_one();
}
