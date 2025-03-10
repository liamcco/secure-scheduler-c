#include <stdlib.h>
#include <stdio.h>

#include "scheduler.h"
#include "task.h"
#include "priority.h"

Scheduler *init_scheduler_fp(void)
{
    Scheduler *scheduler = (Scheduler *)malloc(sizeof(Scheduler));
    scheduler->idle_task = init_idle_task();

    scheduler->compare = &RM;
    scheduler->ready_tasks = NULL;
    scheduler->previous_task = NULL;

    scheduler->schedule_task = &schedule_task_fp;
    scheduler->task_completed = &task_completed_fp;
    scheduler->task_arrived = &task_arrived_fp;
    scheduler->time_step_scheduler = &time_step_scheduler_fp;
    scheduler->load_tasks_scheduler = &load_tasks_scheduler_fp;

    return scheduler;
}

Scheduler *init_scheduler_fp_custom(int (*comp)(const void *, const void *))
{
    Scheduler *scheduler = init_scheduler_fp();
    scheduler->compare = comp;
    return scheduler;
}

void free_scheduler(Scheduler *scheduler)
{
    free(scheduler->idle_task);
    if (scheduler->ready_tasks)
        free(scheduler->ready_tasks);

    free(scheduler);
}

void load_tasks_scheduler_fp(Scheduler *scheduler, Task **tasks, int num_tasks)
{
    scheduler->tasks = tasks;
    scheduler->num_tasks = num_tasks;

    prioritize(tasks, num_tasks, &RM);
}

Task *schedule_task_fp(Scheduler *scheduler)
{
    for (int i = 0; i < scheduler->num_tasks; i++)
    {
        if (is_ready(scheduler->tasks[i]))
        {
            return scheduler->tasks[i];
        }
    }
    return scheduler->idle_task;
}

void task_completed_fp(__attribute__((unused)) Scheduler *scheduler, __attribute__((unused)) Task *task)
{
    return;
}

void task_arrived_fp(__attribute__((unused)) Scheduler *scheduler, __attribute__((unused)) Task *task)
{
    return;
}

void time_step_scheduler_fp(__attribute__((unused)) Scheduler *scheduler)
{
    return;
}
