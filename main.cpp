#include <chrono>
#include <iostream>

#include "easylogging++.h"
#include "TaskSchedulerImpl.h"
#include "Randomizer.h"

INITIALIZE_EASYLOGGINGPP

void ScheduleTask(const std::unique_ptr<ITaskScheduler>& taskScheduler, Randomizer& randomizer) {
  auto task = [duration = randomizer.GetAndPopTaskDuration()]() {
    LOG(INFO) << "Task running. Estimated time of running is: " << duration;

    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
  };

  auto callback = []() { LOG(INFO) << "The callback of task"; };

  taskScheduler->Schedule(task, randomizer.GetAndPopTaskDelay(), randomizer.GetAndPopTaskPriority(),
                          callback);
}

int main(int, char**) {
  LOG(INFO) << "Running Task Scheduler project";

  Randomizer randomizer(10);

  std::unique_ptr<ITaskScheduler> taskScheduler = std::make_unique<TaskSchedulerImpl>();
  for (auto i = 0; i < 1; ++i) {
    ScheduleTask(taskScheduler, randomizer);
  }
  taskScheduler->Start();

  std::this_thread::sleep_for(std::chrono::seconds(5));

  taskScheduler->Stop();

  return 0;
}
