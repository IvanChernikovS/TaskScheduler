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
    explicit TaskSchedulerImpl() : taskIdCounter(0), pool(4), isRunning(false) {
        std::cout << "TaskScheduler::TaskScheduler(): TaskScheduler was created" << std::endl;
    }

    int Schedule(std::function<void()> task, int delay, int priority, std::function<void()> callback) {
        std::lock_guard<std::mutex> lock(mtx);

        int id = ++taskIdCounter;
        auto newTask = std::make_unique<Task>(id, task, callback, priority, delay);
        taskIds.emplace(newTask->taskId);
        taskQueue.push(std::move(newTask));

        std::cout << "TaskScheduler::Schedule(): Task with id {" << id << "} and priority {" << priority << "} scheduled" << std::endl;

        cv.notify_all();

        return id;
    }

    void Cancel(int taskId) {
        std::lock_guard<std::mutex> lock(mtx);

        if (taskIds.find(taskId) != taskIds.end()) {
            // TODO: cancel task in pool

            taskIds.erase(taskId);

            std::cout << "TaskScheduler::Cancel(): Task with id {" << taskId << "} canceled" << std::endl;
        }
    }

    std::vector<int> GetIncompleteTaskIds() {
        std::lock_guard<std::mutex> lock(mtx);

        std::vector<int> incompleteTaskIds;
        for (const auto& id : taskIds) {
            incompleteTaskIds.push_back(id);
        }

        std::cout << "TaskScheduler::GetIncompleteTasks(): There are {" << incompleteTaskIds.size() << "} incomplete tasks" << std::endl;

        return incompleteTaskIds;
    }

    int GetEstimatedStartTime(int taskId) {
        // This function is placeholder since the estimation of start time is a complex problem
        // and depends on many factors such as task duration, priority, etc.
        std::cout << "TaskScheduler::getEstimatedStartTime(): Estimating start time for task id {" << taskId << "}" << std::endl;
        return -1;
    }

    void Start() {
        isRunning = true;
        std::cout << "TaskScheduler::Start(): Starting TaskScheduler" << std::endl;
        while(isRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::lock_guard<std::mutex> lock(mtx);
            while(!taskQueue.empty() && taskQueue.top()->startTime <= std::chrono::system_clock::now()){
                auto task = taskQueue.top();
                pool.Enqueue(task);
                taskQueue.pop();
                taskIds.erase(task->taskId);

                std::cout << "TaskScheduler::Start(): Task with id {" << task->taskId << "} and priority {" << task->priority << "} started" << std::endl;
            }
        }
    }

    void Stop() {
        isRunning = false;
        std::cout << "TaskScheduler::Stop(): Stopping TaskScheduler" << std::endl;
    }
};
