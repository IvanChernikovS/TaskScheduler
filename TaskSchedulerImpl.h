//
// Created by ichernikov on 20.11.23.
//

#pragma once

#include <iostream>
#include <condition_variable>
#include <mutex>
#include <unordered_set>
#include <queue>

#include "Task.h"
#include "ThreadPool.h"
#include "ITaskScheduler.h"

class TaskSchedulerImpl : public ITaskScheduler {
private:
    int taskIdCounter;
    std::priority_queue<std::shared_ptr<Task>> taskQueue;
    std::unordered_set<int> taskIds;
    std::condition_variable cv;
    std::mutex mtx;
    ThreadPool pool;
    bool isRunning;

public:
    explicit TaskSchedulerImpl() : taskIdCounter(0), pool(10), isRunning(false) {
        LOG(INFO) << "TaskScheduler::TaskScheduler(): TaskScheduler was created";
    }

    int Schedule(std::function<void()> task, int delay, int priority, std::function<void()> callback) override {
        std::lock_guard<std::mutex> lock(mtx);

        int id = ++taskIdCounter;
        auto newTask = std::make_unique<Task>(id, task, callback, priority, delay);
        taskIds.emplace(newTask->taskId);
        taskQueue.push(std::move(newTask));

        LOG(INFO) << "TaskScheduler::Schedule(): Task with ID: " << id << " and priority: " << priority << " scheduled";

        cv.notify_all();

        return id;
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
        for (const auto &id: taskIds) {
            incompleteTaskIds.push_back(id);
        }

        LOG(INFO) << "TaskScheduler::GetIncompleteTasks(): There are " << incompleteTaskIds.size()
                  << " incomplete tasks" << std::endl;

        return incompleteTaskIds;
    }

    int GetEstimatedStartTime(int taskId) override {
        LOG(INFO) << "TaskScheduler::getEstimatedStartTime(): Estimating start time for task ID: " << taskId;
        return -1;
    }

    void Start() override {
        isRunning = true;
        LOG(INFO) << "TaskScheduler::Start(): Starting TaskScheduler";
        while (isRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::lock_guard<std::mutex> lock(mtx);
            while (!taskQueue.empty() && taskQueue.top()->startTime <= std::chrono::system_clock::now()) {
                auto task = taskQueue.top();
                pool.Enqueue(task);
                taskQueue.pop();
                taskIds.erase(task->taskId);

                LOG(INFO) << "TaskScheduler::Start(): Task with ID: " << task->taskId << " and priority: "
                          << task->priority << " started";
            }
        }
    }

    void Stop() override {
        isRunning = false;
        LOG(INFO) << "TaskScheduler::Stop(): Stopping TaskScheduler";
    }
};
