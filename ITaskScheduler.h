//
// Created by ichernikov on 21.11.23.
//

#pragma once

#include <vector>
#include <functional>

class ITaskScheduler {
public:
    virtual ~ITaskScheduler() = default;

    virtual int Schedule(std::function<void()> task, int delay, int priority, std::function<void()> callback) = 0;
    virtual void Cancel(int taskId) = 0;
    virtual std::vector<int> GetIncompleteTaskIds() = 0;
    virtual int GetEstimatedStartTime(int taskId) = 0;
    virtual void Start() = 0;
};