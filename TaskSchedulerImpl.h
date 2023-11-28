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
  std::mutex mtx;
  ThreadPool pool;
  volatile bool isRunning;

 public:
  explicit TaskSchedulerImpl() : taskIdCounter(0), pool(10), isRunning(false) {
    LOG(INFO) << "TaskScheduler::TaskScheduler(): TaskScheduler was created";
  }

  ~TaskSchedulerImpl() noexcept override {
    isRunning = false;
    cv.notify_all();
  }

  int Schedule(std::function<void()>&& task, int delay, int priority,
               std::function<void()>&& callback) override {
    std::lock_guard<std::mutex> lock(mtx);

    int id = ++taskIdCounter;
    auto newTask =
        std::make_unique<Task>(id, std::move(task), std::move(callback), priority, delay);
    taskIds.emplace(newTask->taskId);
    taskQueue[priority].emplace_back(std::move(newTask));
    // taskQueue.push(std::move(newTask));

    LOG(INFO) << "TaskScheduler::Schedule(): Task with ID: " << id << " and priority: " << priority
              << " scheduled";

    cv.notify_all();

    return id;
  }

  int ScheduleCompletingTask() override {
    auto task = []() { LOG(INFO) << "Running completing task."; };
    auto callback = [this]() {
      if (!taskQueue.empty()) {
        LOG(INFO) << "Task queue is not empty. Scheduling completing task again.";
        ScheduleCompletingTask();
        return;
      }
      isRunning = false;
      LOG(INFO) << "The callback of completing task.";
      cv.notify_all();
    };

    return Schedule(task, 1000, 10, callback);
  }

  void Cancel(int taskId) override {
    std::lock_guard<std::mutex> lock(mtx);

    if (taskIds.find(taskId) != taskIds.end()) {
      // TODO: cancel task in pool

      taskIds.erase(taskId);

      LOG(INFO) << "TaskScheduler::Cancel(): Task with ID: " << taskId << " canceled";
    }
  }

  std::vector<int> GetIncompleteTaskIds() override {
    std::lock_guard<std::mutex> lock(mtx);

    std::vector<int> incompleteTaskIds;
    for (const auto& id : taskIds) {
      incompleteTaskIds.push_back(id);
    }

    LOG(INFO) << "TaskScheduler::GetIncompleteTasks(): There are " << incompleteTaskIds.size()
              << " incomplete tasks" << std::endl;

    return incompleteTaskIds;
  }

  int GetEstimatedStartTime(int taskId) override {
    LOG(INFO) << "TaskScheduler::getEstimatedStartTime(): Estimating start time for task ID: "
              << taskId;
    return -1;
  }

  void Start() override {
    if (isRunning) {
      LOG(ERROR) << "TaskScheduler::Start(): Task scheduler is running already";
      return;
    }

    isRunning = true;
    LOG(INFO) << "TaskScheduler::Start(): Starting TaskScheduler";
    while (isRunning) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      std::lock_guard<std::mutex> lock(mtx);
      for (auto& tasks : taskQueue) {
        for (auto& task : tasks.second) {
          if (task && task->startTime <= std::chrono::system_clock::now()) {
            pool.Enqueue(task);
            taskIds.erase(task->taskId);
            task = nullptr;
          }
        }
      }

      ClearTaskQueue();
    }
  }

  void Stop() override {
    if (!isRunning) {
      LOG(ERROR) << "TaskScheduler::Stop(): TaskScheduler has been stopped already";
    }

    isRunning = false;
    LOG(INFO) << "TaskScheduler::Stop(): Stopping TaskScheduler";

    cv.notify_all();
  }

 private:
  void ClearTaskQueue() {
    for (auto it = taskQueue.begin(); it != taskQueue.end();) {
      if (it->second.empty() || std::all_of(it->second.cbegin(), it->second.cend(),
                                            [](const auto& task) { return !task; })) {
        it = taskQueue.erase(it);
      } else {
        ++it;
      }
    }
  }
};
