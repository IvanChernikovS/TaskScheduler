#include <chrono>
#include <iostream>

#include "easylogging++.h"
#include "Randomizer.h"
#include "TaskSchedulerImpl.h"

INITIALIZE_EASYLOGGINGPP

void ScheduleTask(const std::unique_ptr<ITaskScheduler>& taskScheduler, Randomizer& /*randomizer*/) {
  auto task = [] { LOG(INFO) << "Task running."; };
  auto callback = [] { LOG(INFO) << "Callback running"; };

  taskScheduler->Schedule(task, 0, callback);
}

int ScheduleCompletingTask(const std::unique_ptr<ITaskScheduler>& taskScheduler) {
  auto task = [&taskScheduler] {
    LOG(INFO) << "Running completing task";
    taskScheduler->Stop();
  };
  auto callback = [] { LOG(INFO) << "The callback of completing task"; };

  return taskScheduler->Schedule(task, 0, callback);
}

void ShutdownTaskScheduler(int delayInSeconds, const std::unique_ptr<ITaskScheduler>& taskScheduler) {
  std::thread t([delayInSeconds, &taskScheduler]() {
    std::this_thread::sleep_for(std::chrono::seconds(delayInSeconds));
    ScheduleCompletingTask(taskScheduler);
  });
  t.detach();
}

int main(int, char**) {
  LOG(INFO) << "Running Task Scheduler project";

  Randomizer randomizer(10);

  std::unique_ptr<ITaskScheduler> taskScheduler = std::make_unique<TaskSchedulerImpl>();
  for (auto i = 0; i < 10000; ++i) {
    ScheduleTask(taskScheduler, randomizer);
  }

  const auto delayForShutdownInSec = 1;
  ShutdownTaskScheduler(delayForShutdownInSec, taskScheduler);
  taskScheduler->Start();

  return 0;
}
