#include <chrono>
#include <iostream>
#include <random>

#include "easylogging++.h"
#include "TaskSchedulerImpl.h"

INITIALIZE_EASYLOGGINGPP

std::mt19937 GetRandomizer() {
  std::random_device randomDevice;
  std::mt19937::result_type seed =
      randomDevice() ^
      ((std::mt19937::result_type)std::chrono::duration_cast<std::chrono::seconds>(
           std::chrono::system_clock::now().time_since_epoch())
           .count() +
       (std::mt19937::result_type)std::chrono::duration_cast<std::chrono::microseconds>(
           std::chrono::high_resolution_clock::now().time_since_epoch())
           .count());

  std::mt19937 gen(seed);

  return gen;
}

class Randomizer {
 private:
  int containerCapacity;

  std::queue<int> taskDurations;
  std::queue<int> taskPriorities;
  std::queue<int> taskDelays;

  std::mt19937 generator;

 public:
  explicit Randomizer(int capacity) : containerCapacity(capacity), generator(GetRandomizer()) {
    FillTaskDurations();
    FillTaskPriorities();
    FillTaskDelays();
  }

  void FillTaskDurations() {
    std::uniform_int_distribution<unsigned> distribution(1, 50);

    for (auto i = 0; i < containerCapacity; ++i) {
      taskDurations.emplace(distribution(generator));
    }
  }

  void FillTaskPriorities() {
    std::uniform_int_distribution<unsigned> distribution(1, 5);

    for (auto i = 0; i < containerCapacity; ++i) {
      taskPriorities.emplace(distribution(generator));
    }
  }

  void FillTaskDelays() {
    std::uniform_int_distribution<unsigned> distribution(1, 5);

    for (auto i = 0; i < containerCapacity; ++i) {
      taskDelays.emplace(distribution(generator) * 200);
    }
  }

  int GetAndPopTaskDuration() {
    const auto duration = taskDurations.front();
    taskDurations.pop();

    return duration;
  }

  int GetAndPopTaskPriority() {
    const auto duration = taskPriorities.front();
    taskPriorities.pop();

    return duration;
  }

  int GetAndPopTaskDelay() {
    const auto duration = taskDelays.front();
    taskDelays.pop();

    return duration;
  }
};

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
