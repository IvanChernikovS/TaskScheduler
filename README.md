# TaskScheduler

## Task
Implement a Multithreading TaskScheduler with Completion Callbacks, Task Status Reporting and Estimated Start Times

In this task, should be built a multithreading task scheduler with advanced features like completion callbacks on the main thread, task status reporting, and estimated start times.

## Requirements

### Task Scheduler Interface
Implement a TaskScheduler class that provides the following interface:

- int Schedule(std::function task, int delay, std::function callback): Schedules a task for execution after delay in milliseconds.
- void Cancel(int taskId): Cancels a previously scheduled task.
- std::vector GetIncompleteTasks(): Returns a list of task IDs for all tasks that have not yet completed.
- int GetEstimatedStartTime(int taskId): Returns the estimated start time (in milliseconds from now) for a given task.

### Main Control Loop
The TaskScheduler should have a main control loop that checks for tasks that are ready to execute and processes task completion callbacks.

### Thread Pool
Implement a thread pool to execute the tasks. When a task is ready to execute, a thread from the pool should be assigned to execute it.

### Thread Safety
The TaskScheduler must be thread-safe. Multiple threads should be able to schedule and cancel tasks at the same time without causing data inconsistencies or crashes.

### Wait Conditions and Mutexes
Main control loop should wait when there are no tasks to execute or callbacks to process. As soon as a new task is scheduled or a task is completed, main loop should wake up to process them.

### Run Loop
The main control loop should be efficient and not consume CPU resources when there are no tasks to execute or callbacks to process.

### Task ID
Each task should be assigned a unique ID when it is scheduled. This ID should be returned by the schedule function and can be used to cancel the task.

# Acceptance Criteria:

### 1. Performance
The TaskScheduler should be able to handle at least 10000 tasks per second using at least 5 different threads.
The system should maintain steady performance even under peak load.

### 2. Functionality and Accuracy
- All tasks should be processed as per their delays. The task with the smallest delay should always be processed first.
- The Schedule() function should return a unique task ID for each task.
- The Cancel() function should successfully cancel tasks that haven't started executing. If a task is already executing, the function should do nothing.
- The GetIncompleteTasks() function should accurately return a list of all tasks that have not yet completed, in the correct order of execution.
- The GetEstimatedStartTime() function should accurately return the estimated start time for a given task (in milliseconds from now). If the task does not exist or has already started, the function should return -1.

### 3. Multithreading and Thread Safety
The TaskScheduler should use a thread pool to execute tasks. When a task is ready to execute, a thread from the pool should be assigned to execute it.
Multiple threads should be able to schedule and cancel tasks at the same time without causing data inconsistencies or crashes.

### 4. Callbacks and Task Reporting
The completion callback function should be called on the main thread when a task has finished executing.
All task completions should be reported via the callback function.

### 5. Efficiency and CPU Usage
The main control loop should be efficient and not consume CPU resources when there are no tasks to execute or callbacks to process.

# Testing

To test the implementation, should be created a program that schedules a number of tasks with different delays, and then attempts to cancel some of them before they execute.
The tasks themselves could be simple functions that print a message to the console.
The callback function could also print a message to the console indicating that the task has finished.
The TaskScheduler should execute the tasks in the correct order, cancelled tasks should not be executed, all task completions should be reported via the callback function, and the list of incomplete tasks and their estimated start times should be correctly reported.
