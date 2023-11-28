//
// Created by ichernikov on 28.11.23.
//

#pragma once

#include <random>
#include <queue>

class Randomizer {
 private:
  int containerCapacity;

  std::queue<int> taskDurations;
  std::queue<int> taskPriorities;
  std::queue<int> taskDelays;

  std::mt19937 generator;

 public:
  explicit Randomizer(int capacity);

  void FillTaskDurations();
  void FillTaskPriorities();
  void FillTaskDelays();

  int GetAndPopTaskDuration();
  int GetAndPopTaskPriority();
  int GetAndPopTaskDelay();
};
