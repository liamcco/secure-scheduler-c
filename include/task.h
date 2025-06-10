#ifndef TASK_H
#define TASK_H

// Define the Task struct
typedef struct Task
{
    int period;
    int duration;
    int deadline;
    double utilization;
    int remaining_deadline;
    int time_until_next_period;
    int remaining_execution_time;
    int max_jitter;
    int remaining_jitter;
    int priority;
    int id;
    int idx;

    int (*is_ready)(struct Task *self);
    int (*is_fresh)(struct Task *self);
    int (*is_complete)(struct Task *self);
    int (*is_new)(struct Task *self);
    void (*reset)(struct Task *self);
    int (*execute)(struct Task *self);
    int (*time_step)(struct Task *self);

    int maximum_inversion_budget;
    int remaining_inversion_budget;
    int c_id;
    int c_idx;

    int trusted;

} Task;

// Macro to allow calling execute like a method
#define execute(task) (task)->execute(task)
#define time_step(task) (task)->time_step(task)
#define reset(task) (task)->reset(task)
#define is_ready(task) (task)->is_ready(task)
#define is_fresh(task) (task)->is_fresh(task)
#define is_complete(task) (task)->is_complete(task)
#define is_new(task) (task)->is_new(task)

// Function prototypes
Task *init_task(int period, int duration);
Task *init_idle_task(void);
void free_tasks(Task **task, int num_tasks);
int task_is_ready(Task *task);
int task_is_fresh(Task *task);
int task_is_complete(Task *task);
int task_is_new(Task *task);
void reset_task(Task *task);
int execute_task(Task *task);
int time_step_task(Task *task);
int gcd(int a, int b);

#endif // TASK_H
