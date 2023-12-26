//
// Created by ichernikov on 27.12.23.
//

#include "ThreadPool.h"

#include "easylogging++.h"
#include "Task.h"

ThreadPool::ThreadPool(int threads) : stop(false) {
  LOG(INFO) << "ThreadPool::ThreadPool(): Creating ThreadPool with " << threads << " threads";

  for (auto i = 0; i < threads; ++i)
    workers.emplace_back([self = this] {
      for (;;) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(self->mutex);
          self->cv.wait(lock, [predSelf = self] { return predSelf->stop.load() || !predSelf->tasks.empty(); });
          if (self->stop.load() && self->tasks.empty()) {
            LOG(INFO) << "ThreadPool::ThreadPool(): Worker with thread id:  " << std::this_thread::get_id()
                      << " is stopping. Exiting thread";
            return;
          }
          task = std::move(self->tasks.front());
          self->tasks.pop();
        }

        LOG(INFO) << "ThreadPool::ThreadPool(): Worker with thread id: " << std::this_thread::get_id()
                  << " is running a task";
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
    std::lock_guard<std::mutex> lock(mutex);
    tasks.emplace([task, self = this] {
      LOG(INFO) << "ThreadPool::Enqueue(): Running task with ID: " << task->taskId;
      task->task();

      self->completedTaskIds.emplace_back(task->taskId);
    });
  }
  cv.notify_one();
}

std::vector<int> ThreadPool::GetCompletedTaskIds() {
  std::lock_guard<std::mutex> lock(mutex);
  const auto ids = completedTaskIds;
  completedTaskIds.clear();

  return ids;
}
