//
// Created by ichernikov on 27.12.23.
//

#include "TaskSchedulerImpl.h"

#include "easylogging++.h"
#include "Task.h"
#include "ThreadPool.h"

namespace {
inline std::chrono::steady_clock::time_point GetEndlessTimePoint() {
  return std::chrono::steady_clock::now() + std::chrono::minutes(std::numeric_limits<int>::max());
}
} // namespace

TaskSchedulerImpl::TaskSchedulerImpl(std::unique_ptr<IThreadPool> thread_pool)
    : taskIdCounter(0), wakeUpTime(GetEndlessTimePoint()), isRunning(false), pool(std::move(thread_pool)) {
  LOG(INFO) << "TaskScheduler::TaskScheduler(): TaskScheduler was created";
}

TaskSchedulerImpl::~TaskSchedulerImpl() noexcept {
  LOG(INFO) << "TaskScheduler::~TaskScheduler(): Destroying";
  pool->DetachTaskScheduler();
  isRunning.store(false);
  cv.notify_one();
}

int TaskSchedulerImpl::Schedule(std::function<void()>&& task, int delay, std::function<void()>&& callbackToStore) {
  std::lock_guard<std::mutex> lock(mutex);

  int id = ++taskIdCounter;
  auto newTask = std::make_unique<Task>(id, std::move(task), std::move(callbackToStore), delay);
  const auto taskStartTime = newTask->startTime;

  taskQueue.emplace(taskStartTime, std::move(newTask));

  UpdateWakeUpTime();

  LOG(INFO)
      << "TaskScheduler::Schedule(): Task with ID: " << id << " and start time in: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(taskStartTime - std::chrono::steady_clock::now()).count()
      << " ms";

  cv.notify_one();

  return id;
}

void TaskSchedulerImpl::CancelTask(int taskId) {
  std::lock_guard<std::mutex> lock(mutex);

  const auto taskIt = std::find_if(taskQueue.cbegin(), taskQueue.cend(),
                                   [taskId](const auto& pair) { return pair.second->taskId == taskId; });

  if (taskIt != taskQueue.end()) {
    taskQueue.extract(taskIt);
    LOG(INFO) << "TaskScheduler::Cancel(): Task with ID: " << taskId << " canceled";
  }
}

std::vector<int> TaskSchedulerImpl::GetIncompleteTaskIds() const {
  std::lock_guard<std::mutex> lock(mutex);

  std::vector<int> incompleteTaskIds;
  incompleteTaskIds.reserve(taskQueue.size());

  for (const auto& pair : taskQueue) {
    incompleteTaskIds.emplace_back(pair.second->taskId);
  }

  return incompleteTaskIds;
}

long TaskSchedulerImpl::GetEstimatedStartTime(int taskId) const {
  std::lock_guard<std::mutex> lock(mutex);
  LOG(INFO) << "TaskScheduler::GetEstimatedStartTime(): Estimating start time for task ID: " << taskId;

  const auto taskIt = std::find_if(taskQueue.cbegin(), taskQueue.cend(),
                                   [taskId](const auto& task) { return task.second->taskId == taskId; });
  if (taskIt == taskQueue.cend()) {
    return -1;
  }

  return (taskIt->first - std::chrono::steady_clock::now()).count();
}

void TaskSchedulerImpl::Start() {
  if (isRunning.load()) {
    LOG(ERROR) << "TaskScheduler::Start(): Task scheduler is running already";
    return;
  }

  isRunning.store(true);
  LOG(INFO) << "TaskScheduler::Start(): Starting TaskScheduler";

  pool->AttachTaskScheduler(this);

  while (isRunning.load()) {
    std::unique_lock<std::mutex> lock(mutex);

    const auto pred = [self = this] {
      return !self->isRunning.load() || !self->taskQueue.empty() || !self->callbacks.empty();
    };
    cv.wait_until(lock, wakeUpTime, pred);

    if (!isRunning.load()) {
      return;
    }

    if (!callbacks.empty()) {
      ExecuteCallbacks();
      UpdateWakeUpTime();
    }

    if (taskQueue.empty()) {
      UpdateWakeUpTime();
      continue;
    }

    if (wakeUpTime > std::chrono::steady_clock::now()) {
      continue;
    }

    pool->Enqueue(taskQueue.cbegin()->second);
    taskQueue.extract(taskQueue.cbegin()->first);

    UpdateWakeUpTime();
  }
}

void TaskSchedulerImpl::Stop() {
  std::lock_guard<std::mutex> lock(mutex);

  if (!isRunning.load()) {
    LOG(ERROR) << "TaskScheduler::Stop(): TaskScheduler has been stopped already";
  }

  ExecuteCallbacks();
  pool->DetachTaskScheduler();

  isRunning.store(false);
  LOG(INFO) << "TaskScheduler::Stop(): Stopping TaskScheduler";

  cv.notify_one();
}

void TaskSchedulerImpl::UpdateCallbacks(std::function<void()>&& callback) {
  callbacks.emplace(std::move(callback));
  LOG(INFO) << "TaskScheduler::UpdateCallbacks(): there are {" << callbacks.size() << "} callbacks";

  cv.notify_one();
}

void TaskSchedulerImpl::UpdateWakeUpTime() {
  wakeUpTime = taskQueue.empty() ? GetEndlessTimePoint() : taskQueue.cbegin()->first;
}

void TaskSchedulerImpl::ExecuteCallbacks() {
  LOG(INFO) << "TaskScheduler::ExecuteCallbacks(): execute {" << callbacks.size() << "} callbacks";

  while (!callbacks.empty()) {
    callbacks.front()();
    callbacks.pop();
  }
}
