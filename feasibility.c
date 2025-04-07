#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "feasibility.h"
#include "math.h"



int RTA(Task **tasks, int num_tasks, int (*compare)(const void *, const void *))
{

    Task **approved_tasks = malloc(num_tasks * sizeof(Task *));
    for (int i = 0; i < num_tasks; i++)
    {
        approved_tasks[i] = tasks[i];
    }

    if (compare)
        qsort(approved_tasks, num_tasks, sizeof(Task *), compare);


    for (int i = 0; i < num_tasks; i++)
    {
        if (!response_time(approved_tasks[i], approved_tasks, i))
        {
            free(approved_tasks);
            return 0;
        }
    }

    free(approved_tasks);
    return 1;
}

int response_time(Task *task, Task **hp_set, int num_tasks)
{
    //printf("Task %d (T=%d): guess=%d WCRT=", task->id, task->period, task->duration);
    int wcrt_guess = task->duration;
    int wcrt;

    while (1)
    {
        wcrt = wcrt_guess;
        wcrt_guess = task->duration + task->max_jitter;
        for (int i = 0; i < num_tasks; i++)
        {
            Task *t = hp_set[i];
            wcrt_guess += ceil((float)(wcrt + t->max_jitter) / (float)t->period) * t->duration;
        }

        if (wcrt_guess > task->deadline)
        {
            //printf(">%d\n", wcrt_guess);
            return 0;
        }

        if (wcrt_guess == wcrt)
        {
            break;
        }
    }

    //printf("%d\n", wcrt);
    return wcrt;
}

int RTA_test_with(TaskGroup *group, Task *task, int (*compare)(const void *, const void *))
{
    Task **tasks = group->tasks;
    int num_tasks = group->num_tasks;

    Task **tasks_copy = malloc((num_tasks + 1) * sizeof(Task *));
    for (int i = 0; i < num_tasks; i++)
    {
        tasks_copy[i] = tasks[i];
    }
    tasks_copy[num_tasks] = task;

    int feasible = RTA(tasks_copy, num_tasks + 1, compare);
    free(tasks_copy);
    return feasible;
}


int LL_test_with(TaskGroup *group, Task *task)
{
    Task **tasks = group->tasks;
    int num_tasks = group->num_tasks;

    double total_U = 0;
    for (int i = 0; i < num_tasks; i++) {
        total_U += tasks[i]->utilization;
    }
    total_U += task->utilization;

    int feasible = total_U < num_tasks*(pow(2,(1.0/(double)num_tasks))-1);

    return feasible;
}
