//
// Created by ichernikov on 27.12.23.
//

#include "TaskSchedulerImpl.h"

#include "easylogging++.h"
#include "Task.h"

namespace {
inline std::chrono::steady_clock::time_point GetTimePointInTheFuture(int delayInMs) {
  return std::chrono::steady_clock::now() + std::chrono::milliseconds(delayInMs);
}
} // namespace

TaskSchedulerImpl::TaskSchedulerImpl()
    : isRunning(false), taskIdCounter(0), wakeUpTime(GetTimePointInTheFuture(300)), pool(10) {
  LOG(INFO) << "TaskScheduler::TaskScheduler(): TaskScheduler was created";
}

TaskSchedulerImpl::~TaskSchedulerImpl() noexcept {
  LOG(INFO) << "TaskScheduler::~TaskScheduler(): Destroying";
  isRunning.store(false);
  cv.notify_one();
}

int TaskSchedulerImpl::Schedule(std::function<void()>&& task, int delay, std::function<void()>&& callback) {
  std::lock_guard<std::mutex> lock(mutex);

  int id = ++taskIdCounter;
  auto newTask = std::make_unique<Task>(id, std::move(task), std::move(callback), delay);
  const auto taskStartTime = newTask->startTime;

  taskQueue.emplace(taskStartTime, std::move(newTask));
  taskIds.emplace(id);

  UpdateWakeUpTime();

  LOG(INFO)
      << "TaskScheduler::Schedule(): Task with ID: " << id << " and start time in: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(taskStartTime - std::chrono::steady_clock::now()).count()
      << " ms";

  cv.notify_one();

  return id;
}

void TaskSchedulerImpl::Cancel(int taskId) {
  std::lock_guard<std::mutex> lock(mutex);

  if (taskIds.find(taskId) != taskIds.end()) {
    const auto startTime = std::find_if(taskQueue.cbegin(), taskQueue.cend(), [taskId](const auto& task) {
                             return task.second->taskId == taskId;
                           })->first;

    taskQueue.extract(startTime);
    taskIds.erase(taskId);

    LOG(INFO) << "TaskScheduler::Cancel(): Task with ID: " << taskId << " canceled";
  }
}

std::vector<int> TaskSchedulerImpl::GetIncompleteTaskIds() const {
  std::lock_guard<std::mutex> lock(mutex);

  std::vector<int> incompleteTaskIds;
  incompleteTaskIds.reserve(taskIds.size());

  for (const auto& id : taskIds) {
    incompleteTaskIds.emplace_back(id);
  }

  return incompleteTaskIds;
}

std::vector<int> TaskSchedulerImpl::GetIncompleteCallbacksIds() const {
  std::lock_guard<std::mutex> lock(mutex);

  std::vector<int> callbackIds;
  callbackIds.reserve(callbacks.size());

  for (const auto& pair : callbacks) {
    callbackIds.emplace_back(pair.first);
  }

  return callbackIds;
}

long TaskSchedulerImpl::GetEstimatedStartTime(int taskId) const {
  std::lock_guard<std::mutex> lock(mutex);
  LOG(INFO) << "TaskScheduler::GetEstimatedStartTime(): Estimating start time for task ID: " << taskId;

  return (std::find_if(taskQueue.cbegin(), taskQueue.cend(),
                       [taskId](const auto& task) { return task.second->taskId == taskId; })
              ->first -
          std::chrono::steady_clock::now())
      .count();
}

void TaskSchedulerImpl::Start() {
  if (isRunning.load()) {
    LOG(ERROR) << "TaskScheduler::Start(): Task scheduler is running already";
    return;
  }

  isRunning.store(true);
  LOG(INFO) << "TaskScheduler::Start(): Starting TaskScheduler";

  while (isRunning.load()) {
    std::unique_lock<std::mutex> lock(mutex);

    cv.wait_until(lock, wakeUpTime, [self = this] { return !self->isRunning.load(); });

    if (!isRunning.load()) {
      LOG(INFO) << "TaskScheduler::Start(): taskScheduler stopped";
      return;
    }

    if (taskQueue.empty()) {
      LOG(DEBUG) << "TaskScheduler::Start(): taskQueue is empty";
      UpdateWakeUpTime();
      continue;
    }

    if (wakeUpTime > std::chrono::steady_clock::now()) {
      LOG_IF(!taskQueue.empty(), DEBUG) << "wakeUpTime > now() ";
      continue;
    }

    auto task = taskQueue.cbegin()->second;
    auto id = task->taskId;

    callbacks[id] = task->callback;
    pool.Enqueue(std::move(task));

    taskQueue.extract(taskQueue.cbegin()->first);
    taskIds.erase(id);

    ExecuteCallbacks();

    UpdateWakeUpTime();
  }
}

void TaskSchedulerImpl::Stop() {
  if (!isRunning.load()) {
    LOG(ERROR) << "TaskScheduler::Stop(): TaskScheduler has been stopped already";
  }

  ExecuteCallbacks();

  isRunning.store(false);
  LOG(INFO) << "TaskScheduler::Stop(): Stopping TaskScheduler";

  cv.notify_one();
}

void TaskSchedulerImpl::UpdateWakeUpTime() {
  wakeUpTime = taskQueue.empty() ? GetTimePointInTheFuture(100) : taskQueue.cbegin()->first;
}

void TaskSchedulerImpl::ExecuteCallbacks() {
  const auto ids = pool.GetCompletedTaskIds();

  for (const auto& id : ids) {
    callbacks.at(id)();
    callbacks.extract(id);
  }
}
