//
// Created by ichernikov on 20.11.23.
//

#pragma once

#include <chrono>
#include <functional>

class Task {
 public:
  int taskId;
  int priority;
  std::function<void()> task;
  std::function<void()> callback;
  std::chrono::system_clock::time_point startTime;

  Task(int id, std::function<void()> t, std::function<void()> c, int p, int delay)
      : taskId(id), task(std::move(t)), callback(std::move(c)), priority(p) {
    startTime = std::chrono::system_clock::now() + std::chrono::milliseconds(delay);
    LOG(INFO) << "Task::Task(): Task created. ID: " << taskId << ", Priority: " << priority
              << ", Delay: " << delay << " ms";
  }

  //  bool operator<(const Task& another) const {
  //    LOG(INFO) << "Comparing tasks. Task ID: " << taskId << " with priority: " << priority
  //              << " < than task ID: " << another.taskId << " with priority: " <<
  //              another.priority;
  //
  //    return priority > another.priority;
  //  }
};
