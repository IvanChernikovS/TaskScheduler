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

  Task(int id, std::function<void()> t, std::function<void()> c, int delay);
};
