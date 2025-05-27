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

# Class UML diagram

![diagram](http://www.plantuml.com/plantuml/png/ZLDDZnCn3BtFhuXw6g0zS1sXQb6rXvLO2MaBTpSntQZvg69dMOJuxzIJjDEK0T53El9xVlQpapjYI9ozwxG38hK5EZpfPpJPOL8_EtNwhRRF2S5yZj4fldmMw16E7spWvPYuLdmwgIAPyLLvEKjW4uEnR6Fu2id2pg7IvyX8U4UrRaziXk5R3bgmKNEfR61bqC6FfMfX3SxjG1_Ug458HKf3qEXwIPBq1oFeIxIcm7laXw2ZFpfabCwf5y8e_8AQOn9JwmlvdjXwO3HFCgsjzTXAJuKzs70ooYcwJp4U-xRkTpZWb-FZYJC30f7T1-bc6yl4RmZuYZepRghXENU_khh7QMNd9LwRA-YwRfDkw_ToZD9pxS_s9kHD7UY_6nFfsfBmgi3gpvsN-6NlBQtTVS5sujRI_JWc6_PDzDnSQJVbS6F0Gpfyd79dqOl1lytxR6mqgNDA65Y-vul-pyZBHMuAjazqyNwXLgkrMhnTbDaEygmORA1vflnVNuz1PKBgfaWZMSADQ8aqQgLklP_KtM4msRlV)

```
@startuml
class TaskScheduler {
    -ThreadPool threadPool
    -Map<int, Task> tasks
    -Mutex mutex
    -ConditionVariable condition
    +schedule(std::function task, int delay, std::function callback) : int
    +cancel(int taskId) : void
    +getIncompleteTasks() : std::vector<int>
    +getEstimatedStartTime(int taskId) : int
    +mainControlLoop() : void
    +wakeUpMainLoop() : void
    +assignTaskToThread() : void
    +executeCallback(int taskId) : void
}

class ThreadPool {
    -std::vector<Thread> threads
    +assignTask(Task task) : void
    +wakeUpThread() : void
    +taskComplete(int taskId) : void
}

class Task {
    -int id
    -std::function task
    -int delay
    -std::function callback
    -bool isExecuting
    -bool isComplete
    +run() : void
    +markAsExecuting() : void
    +markAsComplete() : void
}

class Thread {
    -Task currentTask
    +run() : void
    +assignTask(Task task) : void
    +cancel() : void
}

TaskScheduler "1" --> "*" Task : contains
TaskScheduler "1" --> "1" ThreadPool : uses
ThreadPool "1" --> "*" Thread : contains
Thread "1" --> "1" Task : executes
@enduml
```

# Use case UML diagram

![diagram](http://www.plantuml.com/plantuml/png/RP31JWCn34Jl-GeVsKD_0HMjAiG5LMeHvv1ukrQT6697Whz7xQQqI_SMZT_4dbch-M8riMCS34oWq64qY5Gm64bsFfWKU5CixRdtUjo54MFbgpOMz74hmZF1kPCFLgj7UBXOplxkPj3x793lv6SqUCb1qYUZJIxzFz-eKVA64NQd6B2dX9rpimidUfE_AjQsV0dO5Wce2lsqa2H3xva_V3Zgek7Tay7G-b2GSikxQo4J_QHAXtncmQJLqzrXhs8q_Cmh_lkj9Vd6KEsIP8qvriI_)

```
@startuml
left to right direction
actor User
actor TaskScheduler
actor ThreadPool
actor Thread

User --> (Schedule Task)
User --> (Cancel Task)
User --> (Get Incomplete Tasks)
User --> (Get Estimated Start Time)

TaskScheduler --> (Enqueue Task)
TaskScheduler --> (Process Completion Callbacks)
TaskScheduler --> (Wait for Tasks or Callbacks)

ThreadPool --> (Assign Task to Thread)
ThreadPool --> (Notify TaskScheduler)

Thread --> (Execute Task)

@enduml
```

# Sequence UML diagram

![diagram](http://www.plantuml.com/plantuml/png/nLR1Rjim3BthAuYUR20vnCSTXXPPCFHY34YBdLc9gxqgaYVJs_htYnmZIXGfIxC13H10CQd7zvufAhSze-DXemLAjWuUUtAYGyUjR3iq30_OlwnbGshGSQHnXEgBjJhnU07Omyrk3OkRH4Prc56bCkh3ZFhcX9OGdYtCFnwJX0-mzgAAqcjgVo9Jf28y3XAECZncY6q1xvXHDaSvnT6lKYXAmQ9c-8ejmpVhWBSxUc0BP7uCD900W8XxGimlRBqMCYglTreNli8C56bydO54hPzGlfJYhpMUoP139l_yNXMP7Tj4wGZPr_5vCqZMYtmViM2UgEg81sT6426qjTte4g5iW4SBWq7PDxCQhTmnAiVyr6lO1JJlP4Wqy4I0VTy-6rB0TieLC7t2rb1muHaFvg1IejeYWbZ0F6Rizo_jfjEqDxBC58uDU-mKCYqdd_j2JgkY30YAafH8zmHtnd93xbzSg2vneOeWglTteTgxS5PUVOcyEeAetrTUeEHdnBX83wCnF9w2XXm1-gzvXQd0D9f-anpumj6qsYK7hgKO-wosIL0uc85Jtxrv10f5wQaU1_AtmnAD9Br_AncuisCdnb3qbgicCrpoc657jb9UxcVYUoEdNlJ1lhXKz1RDeuPHtHu3NSTcrNExGIOrNjCFxIQSa2jfqGdY5SpMRBkyCzUSXVCjz8bE_rFyfqxwRbjJbCbmTJvSXt2w_sx9g66Z_m00)

```
@startuml
actor User
participant TaskScheduler
participant ThreadPool
participant Thread1 as "Thread 1"
participant Thread2 as "Thread 2"
participant ThreadN as "Thread N"
participant Task

User -> TaskScheduler : Start()
activate TaskScheduler
activate ThreadPool
TaskScheduler -> ThreadPool : AttachTaskScheduler(TaskScheduler)
deactivate ThreadPool
alt Wait for tasks to enqueue
    TaskScheduler -> TaskScheduler : wait()
end
User -> TaskScheduler : Schedule(task, delay, callback)
TaskScheduler -> TaskScheduler : generateTaskId()
TaskScheduler -> Task : create(taskId, task, delay, callback)
activate Task
Task --> TaskScheduler : return task

loop for each thread
    TaskScheduler -> ThreadPool : Enqueue(task)
    activate ThreadPool

    alt Task can be assigned to Thread
        ThreadPool -> Thread1 : run(task)
        activate Thread1
        Thread1 --> ThreadPool : taskComplete(taskId)
        ThreadPool -> TaskScheduler : UpdateCallbacks(callback())
        deactivate Thread1
    else Another Task can be assigned to Thread
        ThreadPool -> Thread2 : run(task)
        activate Thread2
        Thread2 --> ThreadPool : taskComplete(taskId)
        ThreadPool -> TaskScheduler : UpdateCallbacks(callback())
        deactivate Thread2
    else
        ThreadPool -> ThreadN : run(task)
        activate ThreadN
        ThreadN --> ThreadPool : taskComplete(taskId)
        ThreadPool -> TaskScheduler : UpdateCallbacks(callback())
        deactivate ThreadN
    end

    deactivate ThreadPool
end

alt There are any callbacks to execute
    TaskScheduler -> TaskScheduler : ExecuteCallbacks()
else No callbacks to execute or tasks to assign
    TaskScheduler -> TaskScheduler : wait()
end

deactivate TaskScheduler
deactivate Task

User -> TaskScheduler : Cancel(taskId)
activate TaskScheduler
TaskScheduler -> ThreadPool : Cancel(taskId)
deactivate TaskScheduler
activate ThreadPool
ThreadPool -> Thread1 : Cancel(taskId)
deactivate ThreadPool
deactivate TaskScheduler

User -> TaskScheduler : GetIncompleteTasks()
activate TaskScheduler
TaskScheduler --> User : return incompleteTasks
deactivate TaskScheduler

User -> TaskScheduler : GetEstimatedStartTime(taskId)
activate TaskScheduler
TaskScheduler --> User : return estimatedStartTime
deactivate TaskScheduler

User -> TaskScheduler : Stop()
activate TaskScheduler
TaskScheduler -> TaskScheduler : ExecuteCallbacks()
TaskScheduler -> ThreadPool : DetachTaskScheduler()
deactivate TaskScheduler
activate ThreadPool
ThreadPool -> Thread1 : join()
ThreadPool -> Thread2 : join()
ThreadPool -> ThreadN : join()
deactivate ThreadPool
@enduml
```
