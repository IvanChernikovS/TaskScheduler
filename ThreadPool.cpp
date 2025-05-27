//
// Created by ichernikov on 27.12.23.
//

#include "ThreadPool.h"

#include "easylogging++.h"
#include "ITaskScheduler.h"
#include "Task.h"

ThreadPool::ThreadPool(int threads) : stop(false), taskScheduler(nullptr) {
  LOG(INFO) << "ThreadPool::ThreadPool(): Creating ThreadPool with " << threads << " threads";

  for (auto i = 0; i < threads; ++i)
    workers.emplace_back([self = this] {
      for (;;) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(self->mutex);
          self->cv.wait(lock, [predSelf = self] { return predSelf->stop.load() || !predSelf->tasks.empty(); });
          if (self->stop.load()) {
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

void ThreadPool::AttachTaskScheduler(ITaskScheduler* observer) {
  LOG(INFO) << "ThreadPool::AttachTaskScheduler()";
  std::lock_guard<std::mutex> lock(mutex);
  taskScheduler = observer;
}

void ThreadPool::DetachTaskScheduler() {
  LOG(INFO) << "ThreadPool::DetachTaskScheduler()";
  std::lock_guard<std::mutex> lock(mutex);
  taskScheduler = nullptr;
}

void ThreadPool::NotifyTaskScheduler(std::function<void()> callback) {
  static int counter = 0;
  LOG(INFO) << "ThreadPool::NotifyTaskScheduler(): " << ++counter << "-th time";
  std::lock_guard<std::mutex> lock(mutex);

  if (!taskScheduler) {
    LOG(INFO) << "ThreadPool::NotifyTaskScheduler(): taskScheduler is expired";
    return;
  }

  taskScheduler->UpdateCallbacks(std::move(callback));
}

void ThreadPool::Enqueue(const std::shared_ptr<Task>& task) {
  {
    LOG(INFO) << "ThreadPool::Enqueue(): Enqueuing task with ID: " << task->taskId;
    std::lock_guard<std::mutex> lock(mutex);
    tasks.emplace([task, self = this] {
      LOG(INFO) << "ThreadPool::Enqueue(): Running task with ID: " << task->taskId;
      task->task();

      self->NotifyTaskScheduler(task->callback);
    });
  }
  cv.notify_one();
}
