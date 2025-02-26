#include <stdlib.h>
#include <stdio.h>

#include "task.h"

Task *init_task(int period, int duration)
{
    Task *task = (Task *)malloc(sizeof(Task));

    task->max_jitter = period * 0.1;
    task->remaining_jitter = 0;

    task->period = period;
    task->duration = duration;
    task->deadline = period;
    task->utilization = (double)duration / (double)period;

    task->is_ready = &task_is_ready;
    task->is_fresh = &task_is_fresh;
    task->is_complete = &task_is_complete;
    task->is_new = &task_is_new;
    task->reset = &reset_task;
    task->execute = &execute_task;
    task->time_step = &time_step_task;

    reset_task(task);

    return task;
}

void free_tasks(Task **tasks, int num_tasks)
{
    for (int i = 0; i < num_tasks; i++)
    {
        free(tasks[i]);
    }

    free(tasks);
}

int idle_true(__attribute__((unused)) Task *task)
{
    return 1;
}

int idle_false(__attribute__((unused)) Task *task)
{
    return 0;
}

void idle_void(__attribute__((unused)) Task *task)
{
    return;
}

Task *init_idle_task()
{
    Task *task = (Task *)malloc(sizeof(Task));

    task->is_ready = &idle_true;
    task->is_fresh = &idle_false;
    task->is_complete = &idle_false;
    task->is_new = &idle_false;
    task->reset = &idle_void;
    task->execute = &idle_false;
    task->time_step = &idle_false;

    task->id = 0;

    return task;
}

int task_is_ready(Task *task)
{
    return task->remaining_execution_time > 0 && task->remaining_jitter == 0;
}

int task_is_fresh(Task *task)
{
    return task->duration == task->remaining_execution_time;
}

int task_is_complete(Task *task)
{
    return task->remaining_execution_time == 0;
}

int task_is_new(Task *task)
{
    return task->time_until_next_period == task->period;
}

void reset_task(Task *task)
{
    task->remaining_deadline = task->deadline;
    task->time_until_next_period = task->period;
    task->remaining_execution_time = task->duration;

    if (task->max_jitter == 0)
    {
        return;
    }

    task->remaining_jitter = rand() % task->max_jitter;
}

int execute_task(Task *task)
{
    if (task->remaining_execution_time == 0)
    {
        printf("Error: Trying to execute task when it's already complete!\n");
        exit(EXIT_FAILURE);
    }

    task->remaining_execution_time--;

    return task->remaining_execution_time == 0;
}

int time_step_task(Task *task)
{
    if (task->remaining_execution_time > 0)
    {
        task->remaining_deadline--;
    }

    if (task->remaining_deadline == 0)
    {
        printf("Error: Task %d, Deadline missed!\n", task->id);
        exit(EXIT_FAILURE);
    }

    // Always decrement time until next period
    task->time_until_next_period--;

    int taskDidRenew = 0;

    // If time until next period is 0, reset task
    if (task->time_until_next_period == 0)
    {
        reset(task);
        taskDidRenew = 1;
    }

    int taskDidArrive = taskDidRenew && task->remaining_jitter == 0;

    if (task->remaining_jitter > 0)
    {
        task->remaining_jitter--;
        taskDidArrive = task->remaining_jitter == 0;
    }

    return taskDidArrive;
}

// Function to compute the greatest common divisor (GCD)
int gcd(int a, int b)
{
    while (b != 0)
    {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}
