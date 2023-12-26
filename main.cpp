#include <chrono>
#include <iostream>

#include "easylogging++.h"
#include "Randomizer.h"
#include "TaskSchedulerImpl.h"

INITIALIZE_EASYLOGGINGPP

void ScheduleTask(const std::unique_ptr<ITaskScheduler>& taskScheduler, Randomizer& randomizer) {
  auto task = [duration = randomizer.GetAndPopTaskDuration()]() {
    LOG(INFO) << "Task running. Estimated time of running is: " << duration << " ms";

    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
  };

  auto callback = []() { LOG(INFO) << "The callback of task"; };

  taskScheduler->Schedule(task, randomizer.GetAndPopTaskDelay(), callback);
}

int ScheduleCompletingTask(const std::unique_ptr<ITaskScheduler>& taskScheduler) {
  auto task = [&taskScheduler] {
    LOG(INFO) << "Running completing task";
    if (!taskScheduler->GetIncompleteTaskIds().empty() || taskScheduler->GetIncompleteCallbacksIds().size() > 1) {
      LOG(INFO) << "Task scheduler can't finish its work: task queue size {"
                << taskScheduler->GetIncompleteTaskIds().size() << "}, callbacks size: {"
                << taskScheduler->GetIncompleteCallbacksIds().size() << "}";
      ScheduleCompletingTask(taskScheduler);
      return;
    }

    taskScheduler->Stop();
  };
  auto callback = [] { LOG(INFO) << "The callback of completing task"; };

  return taskScheduler->Schedule(task, 1000, callback);
}

int main(int, char**) {
  LOG(INFO) << "Running Task Scheduler project";

  Randomizer randomizer(10);

  std::unique_ptr<ITaskScheduler> taskScheduler = std::make_unique<TaskSchedulerImpl>();
  for (auto i = 0; i < 2; ++i) {
    ScheduleTask(taskScheduler, randomizer);
  }
  ScheduleCompletingTask(taskScheduler);

  taskScheduler->Start();

  return 0;
}
