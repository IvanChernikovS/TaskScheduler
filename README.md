# TaskScheduler

Task: Implement a Multithreading TaskScheduler with Completion Callbacks, Task Status Reporting and Estimated Start Times

In this task, should be built a multithreading task scheduler with advanced features like completion callbacks on the main thread, task status reporting, and estimated start times.

Requirements:

Task Scheduler Interface: Implement a TaskScheduler class that provides the following interface:

int schedule(std::function task, int delay, int priority, std::function callback): Schedules a task for execution after delay milliseconds. The task's priority is specified by priority. The function returns a unique task ID. The callback function is called on the main thread when the task has finished executing.
void cancel(int taskId): Cancels a previously scheduled task. If the task has already started executing, this function should do nothing.
std::vector getIncompleteTasks(): Returns a list of task IDs for all tasks that have not yet completed, in the order they are expected to start executing.
int getEstimatedStartTime(int taskId): Returns the estimated start time (in milliseconds from now) for a given task. If the task does not exist or has already started, this function should return -1.
Main Control Loop: The TaskScheduler should have a main control loop that checks for tasks that are ready to execute and processes task completion callbacks. Use a priority queue to store the tasks, so the task with the highest priority (and the smallest delay) is always at the front of the queue.

Thread Pool: Implement a thread pool to execute the tasks. When a task is ready to execute, a thread from the pool should be assigned to execute it.

Thread Safety: The TaskScheduler must be thread-safe. Multiple threads should be able to schedule and cancel tasks at the same time without causing data inconsistencies or crashes.

Wait Conditions and Mutexes: Use a condition variable to make the main control loop wait when there are no tasks to execute or callbacks to process. As soon as a new task is scheduled or a task is completed, the condition variable should be signaled, waking up the main control loop.

Run Loop: The main control loop should be efficient and not consume CPU resources when there are no tasks to execute or callbacks to process.

Task ID: Each task should be assigned a unique ID when it is scheduled. This ID should be returned by the schedule function and can be used to cancel the task.

To test the implementation, should be created a program that schedules a number of tasks with different delays and priorities, and then attempts to cancel some of them before they execute. The tasks themselves could be simple functions that print a message to the console. The callback function could also print a message to the console indicating that the task has finished. The TaskScheduler should execute the tasks in the correct order, cancelled tasks should not be executed, all task completions should be reported via the callback function, and the list of incomplete tasks and their estimated start times should be correctly reported.
