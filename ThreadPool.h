//
// Created by ichernikov on 20.11.23.
//

#pragma once

#include <iostream>
#include <thread>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>
#include <map>

#include "Task.h"

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

public:
    explicit ThreadPool(int);

    ~ThreadPool() noexcept;

    void Enqueue(const std::shared_ptr<Task> task);
};

ThreadPool::ThreadPool(int threads) : stop(false) {
    std::cout << "ThreadPool::ThreadPool(): Creating ThreadPool with " << threads << " threads" << std::endl;

    for (auto i = 0; i < threads; ++i)
        workers.emplace_back(
                [this, i] {
                    for (;;) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                            if (this->stop && this->tasks.empty()) {
                                std::cout << "ThreadPool::ThreadPool(): Worker " << i
                                          << " is stopping because ThreadPool is stopped and tasks are empty. Exiting thread"
                                          << std::endl;
                                return;
                            }
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }

                        std::cout << "ThreadPool::ThreadPool(): Worker " << i << " is running a task" << std::endl;
                        task();
                    }
                }
        );
}

ThreadPool::~ThreadPool() noexcept {
    std::cout << "ThreadPool::~ThreadPool(): Destroying ThreadPool" << std::endl;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (auto &worker: workers) {
        if (worker.joinable()) {
            worker.join();
            std::cout << "ThreadPool::~ThreadPool(): Joined worker thread with ID: " << worker.get_id() << std::endl;
        }
    }
}

void ThreadPool::Enqueue(const std::shared_ptr<Task> task) {
    std::cout << "ThreadPool::enqueue(): Enqueuing task with ID: " << task->taskId << std::endl;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace([task]() {
            std::cout << "ThreadPool::enqueue(): Running task with ID: " << task->taskId << " and its callback"
                      << std::endl;
            task->task();
            task->callback();
        });
    }
    condition.notify_one();
}
