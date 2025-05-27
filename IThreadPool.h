//
// Created by Ivan Chernikov on 28.05.2025.
//

#pragma once

#include <memory>
#include <functional>

class Task;
class ITaskScheduler;

class IThreadPool {
 public:
  virtual ~IThreadPool() = default;

  virtual void Enqueue(const std::shared_ptr<Task>& task) = 0;
  virtual void AttachTaskScheduler(ITaskScheduler* scheduler) = 0;
  virtual void DetachTaskScheduler() = 0;
};

