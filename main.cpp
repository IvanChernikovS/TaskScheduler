#include <iostream>
#include <chrono>
#include <random>

#include "TaskSchedulerImpl.h"

std::mt19937 GetRandomizer() {
    std::random_device randomDevice;
    std::mt19937::result_type seed = randomDevice() ^ (
            (std::mt19937::result_type)
                    std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::system_clock::now().time_since_epoch()
                    ).count() +
            (std::mt19937::result_type)
                    std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::high_resolution_clock::now().time_since_epoch()
                    ).count());

    std::mt19937 gen(seed);

    return gen;
}

class Randomizer {
private:
    std::queue<int> taskDurations;
    std::queue<int> taskPriorities;
    std::queue<int> taskDelays;
public:
    void FillTaskDurations() {
        std::mt19937 gen = GetRandomizer();
        std::uniform_int_distribution<unsigned> distribution(1, 50);

        for (auto i = 0; i < 50; ++i) {
            taskDurations.emplace(distribution(gen));
        }
    }

    void FillTaskPriorities() {
        std::mt19937 gen = GetRandomizer();
        std::uniform_int_distribution<unsigned> distribution(1, 5);

        for (auto i = 0; i < 50; ++i) {
            taskPriorities.emplace(distribution(gen));
        }
    }

    void FillTaskDelays() {
        std::mt19937 gen = GetRandomizer();
        std::uniform_int_distribution<unsigned> distribution(1, 5);

        for (auto i = 0; i < 50; ++i) {
            taskDelays.emplace(distribution(gen) * 200);
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

void ScheduleTask(const std::unique_ptr<ITaskScheduler> &taskScheduler, Randomizer &randomizer) {
    auto task = [duration = randomizer.GetAndPopTaskDuration()]() {
        std::cout << "Task running. Estimated time of running is:" << duration << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    };

    auto callback = []() {
        std::cout << "The callback of task" << std::endl;
    };

    taskScheduler->Schedule(task, randomizer.GetAndPopTaskDelay(), randomizer.GetAndPopTaskPriority(), callback);
}

int main() {
    using namespace std::chrono_literals;

    Randomizer randomizer;
    randomizer.FillTaskDurations();
    randomizer.FillTaskPriorities();
    randomizer.FillTaskDelays();

    std::unique_ptr<ITaskScheduler> taskScheduler = std::make_unique<TaskSchedulerImpl>();
    for (auto i = 0; i < 10; ++i) {
        ScheduleTask(taskScheduler, randomizer);
    }
    taskScheduler->Start();

    std::this_thread::sleep_for(std::chrono::duration(10s));

    taskScheduler->Stop();

    return 0;
}
