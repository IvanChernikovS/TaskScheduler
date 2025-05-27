#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <atomic>
#include <thread>
#include <chrono>
#include <vector>

#include "../TaskSchedulerImpl.h"
#include "Mocks/MockThreadPool.h"

#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

using namespace std::chrono_literals;
using ::testing::_;
using ::testing::Invoke;

// Testable subclass to expose the mock pool
class TestableScheduler : public TaskSchedulerImpl {
 public:
  explicit TestableScheduler(std::unique_ptr<IThreadPool> mockPool)
      : TaskSchedulerImpl(std::move(mockPool)) {}

  IThreadPool* GetMockPool() {
    return pool.get(); // Accessing protected/private member in derived class
  }
};

class TaskSchedulerTest : public ::testing::Test {
 protected:
  std::unique_ptr<MockThreadPool> mockPool;
  std::unique_ptr<TestableScheduler> scheduler;

  void SetUp() override {
    mockPool = std::make_unique<MockThreadPool>();
    scheduler = std::make_unique<TestableScheduler>(std::move(mockPool));
  }
};

TEST_F(TaskSchedulerTest, ScheduleAddsTask) {
  std::atomic<bool> executed = false;

  EXPECT_CALL(*dynamic_cast<MockThreadPool*>(scheduler->GetMockPool()), Enqueue(_))
      .WillOnce(Invoke([&](const std::shared_ptr<Task>& task) {
        task->task();
        task->callback();
      }));

  int id = scheduler->Schedule([&]() { executed = true; }, 1, [] {});
  auto tasks = scheduler->GetIncompleteTaskIds();

  EXPECT_NE(std::find(tasks.begin(), tasks.end(), id), tasks.end());
  EXPECT_TRUE(executed);
}

TEST_F(TaskSchedulerTest, CancelTaskRemovesIt) {
  EXPECT_CALL(*dynamic_cast<MockThreadPool*>(scheduler->GetMockPool()), Enqueue(_)).Times(0);

  int id = scheduler->Schedule([] {}, 1000, [] {});
  scheduler->CancelTask(id);

  auto tasks = scheduler->GetIncompleteTaskIds();
  EXPECT_EQ(std::find(tasks.begin(), tasks.end(), id), tasks.end());
}

TEST_F(TaskSchedulerTest, GetEstimatedStartTimeIsCorrect) {
  EXPECT_CALL(*dynamic_cast<MockThreadPool*>(scheduler->GetMockPool()), Enqueue(_)).Times(0);

  int id = scheduler->Schedule([] {}, 200, [] {});
  long time = scheduler->GetEstimatedStartTime(id);

  EXPECT_GT(time, 0);
}

TEST_F(TaskSchedulerTest, GetEstimatedStartTimeReturnsMinusOneIfNotFound) {
  long time = scheduler->GetEstimatedStartTime(9999);
  EXPECT_EQ(time, -1);
}

TEST_F(TaskSchedulerTest, CallbacksAreExecuted) {
  std::atomic<bool> flag = false;

  scheduler->UpdateCallbacks([&]() {
    flag = true;
  });

  std::thread t([&]() {
    scheduler->Start();
  });

  std::this_thread::sleep_for(50ms);
  scheduler->Stop();
  t.join();

  EXPECT_TRUE(flag);
}

TEST_F(TaskSchedulerTest, TaskExecutesAndTriggersCallback) {
  std::atomic<bool> taskRan = false;
  std::atomic<bool> callbackRan = false;

  EXPECT_CALL(*dynamic_cast<MockThreadPool*>(scheduler->GetMockPool()), Enqueue(_))
      .WillOnce(Invoke([&](const std::shared_ptr<Task>& task) {
        task->task();
        task->callback();
      }));

  int id = scheduler->Schedule(
      [&]() { taskRan = true; },
      0,
      [&]() { callbackRan = true; }
  );

  std::this_thread::sleep_for(50ms);

  EXPECT_TRUE(taskRan);
  EXPECT_TRUE(callbackRan);
}

TEST_F(TaskSchedulerTest, StartTwiceDoesNotCrash) {
  std::thread t([&]() {
    scheduler->Start();
  });

  std::this_thread::sleep_for(50ms);
  scheduler->Start(); // Should log error, not crash

  scheduler->Stop();
  t.join();

  EXPECT_TRUE(true);
}

TEST_F(TaskSchedulerTest, StopTwiceDoesNotCrash) {
  std::thread t([&]() {
    scheduler->Start();
  });

  std::this_thread::sleep_for(50ms);
  scheduler->Stop();
  scheduler->Stop(); // Should log error, not crash
  t.join();

  EXPECT_TRUE(true);
}
