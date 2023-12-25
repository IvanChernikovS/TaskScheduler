//
// Created by ichernikov on 20.11.23.
//

#pragma once

#include <chrono>
#include <functional>

class Task {
 public:
  int taskId;
  std::function<void()> task;
  std::function<void()> callback;
  std::chrono::steady_clock::time_point startTime;

  Task(int id, std::function<void()> t, std::function<void()> c, int delay)
      : taskId(id), task(std::move(t)), callback(std::move(c)) {
    startTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay);

    LOG(INFO) << "Task::Task(): Task created. ID: " << taskId << ", Delay: " << delay
              << " ms, Start time: " << startTime.time_since_epoch().count();
  }
};
