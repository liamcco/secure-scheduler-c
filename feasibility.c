#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "feasibility.h"

int RTA(Task **tasks, int num_tasks)
{

    Task **approved_tasks = malloc(num_tasks * sizeof(Task *));

    for (int i = 0; i < num_tasks; i++)
    {
        if (response_time(tasks[i], approved_tasks, i))
        {
            approved_tasks[i] = tasks[i];
        }
        else
        {
            return 0;
        }
    }

    return 1;
}

int response_time(Task *task, Task **task_set, int num_tasks)
{
    int wcrt_guess = task->duration;
    int wcrt;

    while (1)
    {
        wcrt = wcrt_guess;
        wcrt_guess = task->duration;
        for (int i = 0; i < num_tasks; i++)
        {
            Task *t = task_set[i];
            wcrt_guess += ceil((float)(wcrt + t->max_jitter) / (float)t->period) * t->duration;
        }

        if (wcrt_guess > task->deadline)
        {
            return 0;
        }

        if (wcrt_guess == wcrt)
        {
            break;
        }
    }

    return wcrt;
}

int RTA_test_with(TaskGroup *group, Task *task)
{
    Task **tasks = group->tasks;
    int num_tasks = group->num_tasks;

    return response_time(task, tasks, num_tasks);
}