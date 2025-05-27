//
// Created by ichernikov on 28.11.23.
//

#pragma once

#include <queue>
#include <random>

class Randomizer {
 private:
  int containerCapacity;

  std::queue<int> taskDurations;
  std::queue<int> taskDelays;

  std::mt19937 generator;

 public:
  explicit Randomizer(int capacity);

  void FillTaskDurations();
  void FillTaskDelays();

  int GetAndPopTaskDuration();
  int GetAndPopTaskDelay();
};
