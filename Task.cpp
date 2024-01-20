//
// Created by ichernikov on 27.12.23.
//

#include "Task.h"

#include "easylogging++.h"

Task::Task(int id, std::function<void()> t, std::function<void()> c, int delay)
    : taskId(id), task(std::move(t)), callback(std::move(c)) {
  startTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay);

  LOG(INFO) << "Task::Task(): Task created. ID: " << taskId << ", Delay: " << delay
            << " ms, Start time: " << startTime.time_since_epoch().count();
}
