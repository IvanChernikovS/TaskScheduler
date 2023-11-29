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

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;

  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  void Enqueue(const std::shared_ptr<Task>&& task);
};

ThreadPool::ThreadPool(int threads) : stop(false) {
  LOG(INFO) << "ThreadPool::ThreadPool(): Creating ThreadPool with " << threads << " threads";

  for (auto i = 0; i < threads; ++i)
    workers.emplace_back([self = this] {
      for (;;) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(self->queue_mutex);
          self->cv.wait(lock, [predSelf = self] { return predSelf->stop.load() || !predSelf->tasks.empty(); });
          if (self->stop.load() && self->tasks.empty()) {
            LOG(INFO) << "ThreadPool::ThreadPool(): Worker with thread id:  "
                      << std::this_thread::get_id() << " is stopping. Exiting thread";
            return;
          }
          task = std::move(self->tasks.front());
          self->tasks.pop();
        }

        LOG(INFO) << "ThreadPool::ThreadPool(): Worker with thread id: "
                  << std::this_thread::get_id() << " is running a task";
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
      LOG(INFO) << "ThreadPool::~ThreadPool(): Joining worker thread with ID: " << worker.get_id();
      worker.join();
    }
  }
}

void ThreadPool::Enqueue(const std::shared_ptr<Task>&& task) {
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
