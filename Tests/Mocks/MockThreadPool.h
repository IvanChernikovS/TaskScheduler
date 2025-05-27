//
// Created by Ivan Chernikov on 28.05.2025.
//

#pragma once

#include <gmock/gmock.h>
#include "../../IThreadPool.h"
#include "../../Task.h"

class MockThreadPool : public IThreadPool {
 public:
  MOCK_METHOD(void, Enqueue, (const std::shared_ptr<Task>& task), (override));
  MOCK_METHOD(void, AttachTaskScheduler, (ITaskScheduler* scheduler), (override));
  MOCK_METHOD(void, DetachTaskScheduler, (), (override));
};
