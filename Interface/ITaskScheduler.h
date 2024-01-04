//
// Created by ichernikov on 21.11.23.
//

#pragma once

#include <functional>
#include <vector>

class ITaskScheduler {
 public:
  virtual ~ITaskScheduler() = default;

  virtual int Schedule(std::function<void()>&& task, int delay, std::function<void()>&& callback) = 0;
  virtual void Cancel(int taskId) = 0;

  virtual long GetEstimatedStartTime(int taskId) const = 0;
  virtual std::vector<int> GetIncompleteTaskIds() const = 0;
  virtual std::vector<int> GetIncompleteCallbacksIds() const = 0;

  virtual void Start() = 0;
  virtual void Stop() = 0;
};
