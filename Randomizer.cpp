//
// Created by ichernikov on 28.11.23.
//

#include "Randomizer.h"

#include <chrono>

namespace {
std::mt19937 GetRandomizer() {
  std::random_device randomDevice;
  std::mt19937::result_type seed =
      randomDevice() ^ ((std::mt19937::result_type)std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count() +
                        (std::mt19937::result_type)std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::high_resolution_clock::now().time_since_epoch())
                            .count());

  std::mt19937 gen(seed);

  return gen;
}
} // namespace

Randomizer::Randomizer(int capacity) : containerCapacity(capacity), generator(GetRandomizer()) {}

void Randomizer::FillTaskDurations() {
  std::uniform_int_distribution<unsigned> distribution(1, 50);

  for (auto i = 0; i < containerCapacity; ++i) {
    taskDurations.emplace(distribution(generator) * 100);
  }
}

void Randomizer::FillTaskDelays() {
  std::uniform_int_distribution<unsigned> distribution(1, 5);

  for (auto i = 0; i < containerCapacity; ++i) {
    taskDelays.emplace(distribution(generator) * 200);
  }
}

int Randomizer::GetAndPopTaskDuration() {
  if (taskDurations.empty()) {
    FillTaskDurations();
  }

  const auto duration = taskDurations.front();
  taskDurations.pop();

  return duration;
}

int Randomizer::GetAndPopTaskDelay() {
  if (taskDelays.empty()) {
    FillTaskDelays();
  }

  const auto duration = taskDelays.front();
  taskDelays.pop();

  return duration;
}