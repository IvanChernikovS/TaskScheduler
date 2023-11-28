//
// Created by ichernikov on 20.11.23.
//

#pragma once

#include <chrono>
#include <functional>
#include <iostream>

class Task {
 public:
  int taskId;
  int priority;
  std::function<void()> task;
  std::function<void()> callback;
  std::chrono::system_clock::time_point startTime;

  Task(int id, std::function<void()> t, std::function<void()> c, int p, int delay)
      : taskId(id), task(t), callback(c), priority(p) {
    startTime = std::chrono::system_clock::now() + std::chrono::milliseconds(delay);
    LOG(INFO) << "Task::Task(): Task created. ID: " << taskId << ", Priority: " << priority
              << ", Delay: " << delay << " ms";
  }

  bool operator<(const Task& other) const {
    if (startTime > other.startTime) {
      LOG(INFO) << "Task::Comparing tasks. ID: " << taskId
                << " starts later than ID: " << other.taskId;
      return true;
    }

    LOG(INFO) << "Comparing tasks. ID: " << taskId
              << " starts earlier or at the same time as ID: " << other.taskId;
    return false;
  }
};
