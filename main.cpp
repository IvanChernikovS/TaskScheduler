#include <chrono>

#include "easylogging++.h"
#include "TaskSchedulerImpl.h"

INITIALIZE_EASYLOGGINGPP

namespace {
// to make a function load
int Factorial(int n) {
  if (n == 0 || n == 1) return 1;
  return n * Factorial(n - 1);
}

void ScheduleTask(const std::unique_ptr<ITaskScheduler>& taskScheduler) {
  auto task = []() { LOG(INFO) << "Task running. Factorial == " << Factorial(15); };
  auto callback = []() { LOG(INFO) << "Task completed"; };

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
    taskScheduler->Stop();
  });
  t.detach();
}

void StartTaskSchedulerWithTimeMeasure(const std::unique_ptr<ITaskScheduler>& taskScheduler) {
  using std::chrono::duration;
  using std::chrono::duration_cast;
  using std::chrono::high_resolution_clock;
  using std::chrono::milliseconds;

  auto startTime = high_resolution_clock::now();
  taskScheduler->Start();
  auto endTime = high_resolution_clock::now();
  auto timeDuration = duration_cast<milliseconds>(endTime - startTime);
  LOG(INFO) << "Time of processing tasks: " << timeDuration.count() << "ms";
}
} // namespace

int main(int, char**) {
  std::unique_ptr<ITaskScheduler> taskScheduler = std::make_unique<TaskSchedulerImpl>();
  for (auto i = 0; i < 10000; ++i) {
    ScheduleTask(taskScheduler);
  }

  const auto delayForShutdownInSec = 1;
  ShutdownTaskScheduler(delayForShutdownInSec, taskScheduler);

  StartTaskSchedulerWithTimeMeasure(taskScheduler);

  return 0;
}
