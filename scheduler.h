#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct Scheduler
{
    int hyperperiod;
    Task **tasks;
    int num_tasks;
    Task *idle_task;

    int (*compare)(const void *, const void *);

    int to_schedule;
    Task *previous_task;

    int push_back;

    int risat_budget;

    int adjust_budget;

    Task *(*schedule_task)(struct Scheduler *scheduler);
    void (*task_completed)(struct Scheduler *scheduler, Task *task);
    void (*task_arrived)(struct Scheduler *scheduler, Task *task);
    void (*time_step_scheduler)(struct Scheduler *scheduler);
    void (*load_tasks_scheduler)(struct Scheduler *scheduler, Task **tasks, int num_tasks);
} Scheduler;

// Macro to allow calling execute like a method
#define schedule_task(scheduler) ((scheduler)->schedule_task(scheduler))
#define task_completed(scheduler, task) ((scheduler)->task_completed(scheduler, task))
#define task_arrived(scheduler, task) ((scheduler)->task_arrived(scheduler, task))
#define time_step_scheduler(scheduler) ((scheduler)->time_step_scheduler(scheduler))
#define load_tasks_scheduler(scheduler, tasks, num_tasks) ((scheduler)->load_tasks_scheduler(scheduler, tasks, num_tasks))

Scheduler *init_scheduler_fp(void);
Scheduler *init_scheduler_fp_custom(int (*comp)(const void *, const void *));
Scheduler *init_scheduler_ts(void);
Scheduler *init_scheduler_ts_custom(int (*comp)(const void *, const void *));

void free_scheduler(Scheduler *scheduler);

Task *schedule_task_fp(Scheduler *scheduler);
void task_completed_fp(Scheduler *scheduler, Task *task);
void task_arrived_fp(Scheduler *scheduler, Task *task);
void time_step_scheduler_fp(Scheduler *scheduler);
void load_tasks_scheduler_fp(Scheduler *scheduler, Task **tasks, int num_tasks);
void load_tasks_scheduler_mid(Scheduler *scheduler, Task **tasks, int num_tasks);

Task *schedule_task_ts(Scheduler *scheduler);
Task *pick_task(Scheduler *scheduler);
int minimum_inversion_priority(int c_idx, Task **tasks, int num_tasks);
int next_schedule_decision_to_be_made(Task **ready_tasks, int idx);
int next_schedule_decision_to_be_made_idle(Task **ready_tasks, int num_ready_tasks);
void task_completed_ts(Scheduler *scheduler, Task *task);
void task_arrived_ts(Scheduler *scheduler, Task *task);
void time_step_scheduler_ts(Scheduler *scheduler);
void load_tasks_scheduler_ts(Scheduler *scheduler, Task **tasks, int num_tasks);
void decrement_task_budgets(Scheduler *scheduler);
int worst_case_maximum_inversion_budget(Task *task, Task **tasks);
int upper_bound_interference_from_hp(Task *task_i, Task **tasks);
int worst_case_maximum_inversion_budget_risat(Task *task, Task **tasks);
int upper_bound_interference_from_hp_risat(Task *task_i, Task **tasks);

#endif // SCHEDULER_H
