#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "feasibility.h"
#include "math.h"
#include "opa.h"


int RTA_custom(Task **tasks, int num_tasks, int (*compare)(const void *, const void *), int migration)
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
        if (!response_time_custom(approved_tasks[i], approved_tasks, i, migration))
        {
            free(approved_tasks);
            return 0;
        }
    }

    free(approved_tasks);
    return 1;
}

int RTA(Task **tasks, int num_tasks, int (*compare)(const void *, const void *))
{
    return RTA_custom(tasks, num_tasks, compare, 0);
}

int RTA_migration(Task **tasks, int num_tasks, int (*compare)(const void *, const void *))
{
    return RTA_custom(tasks, num_tasks, compare, 1);
}

int response_time_custom(Task *task, Task **hp_set, int num_tasks, int migration)
{
    //printf("Task %d (T=%d): guess=%d WCRT=", task->id, task->period, task->duration);
    int wcrt_guess = task->remaining_execution_time;
    int wcrt;

    int base;
    if (migration)
        base = task->remaining_execution_time;
    else
        base = task->duration + task->max_jitter;

    int deadline;
    if (migration)
        deadline = task->remaining_deadline;
    else
        deadline = task->deadline;

    while (1)
    {
        wcrt = wcrt_guess;
        wcrt_guess = base;
        for (int i = 0; i < num_tasks; i++)
        {
            Task *t = hp_set[i];
            wcrt_guess += ceil((float)(wcrt + t->max_jitter) / (float)t->period) * t->duration;
        }

        if (wcrt_guess > deadline)
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

int response_time(Task *task, Task **hp_set, int num_tasks)
{
    return response_time_custom(task, hp_set, num_tasks, 0);
}

int response_time_migration(Task *task, Task **hp_set, int num_tasks)
{
    return response_time_custom(task, hp_set, num_tasks, 1);
}

int RTA_test_with_custom(TaskGroup *group, Task *task, int (*compare)(const void *, const void *), int migration)
{
    Task **tasks = group->tasks;
    int num_tasks = group->num_tasks;

    Task **tasks_copy = malloc((num_tasks + 1) * sizeof(Task *));
    for (int i = 0; i < num_tasks; i++)
    {
        tasks_copy[i] = tasks[i];
    }
    tasks_copy[num_tasks] = task;

    int feasible = RTA_custom(tasks_copy, num_tasks + 1, compare, migration);
    
    free(tasks_copy);
    return feasible;
}

int RTA_test_with_custom_OPA(TaskGroup *group, Task *task, int (*compare)(const void *, const void *), int migration)
{
    Task **tasks = group->tasks;
    int num_tasks = group->num_tasks;

    Task **tasks_copy = malloc((num_tasks + 1) * sizeof(Task *));
    for (int i = 0; i < num_tasks; i++)
    {
        tasks_copy[i] = tasks[i];
    }
    tasks_copy[num_tasks] = task;

    if (compare)
        OPA_with_priority(tasks_copy, num_tasks + 1, compare);

    int feasible = RTA_custom(tasks_copy, num_tasks + 1, NULL, migration);
    
    free(tasks_copy);
    return feasible;
}

int RTA_test_with_migration(TaskGroup *group, Task *task, int (*compare)(const void *, const void *))
{
    return RTA_test_with_custom(group, task, compare, 1);
}

int RTA_test_with(TaskGroup *group, int (*compare)(const void *, const void *))
{
    return RTA_test_with_custom(group, NULL, compare, 0);
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

    int feasible = total_U < (num_tasks+1)*(pow(2,(1.0/(double)(num_tasks+1)))-1);

    return feasible;
}
