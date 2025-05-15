#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "feasibility.h"
#include "math.h"
#include "opa.h"
#include "priority.h"
#include "task.h"


int RTA_custom(Task **tasks, int num_tasks, int (*compare)(const void *, const void *), int migration)
{
    //printf("Now performing RTA on set with %d tasks\n", num_tasks);
    Task **approved_tasks = malloc(num_tasks * sizeof(Task *));
    for (int i = 0; i < num_tasks; i++)
    {
        approved_tasks[i] = tasks[i];
    }

    if (compare)
        qsort(approved_tasks, num_tasks, sizeof(Task *), compare);

    for (int i = 0; i < num_tasks; i++)
    {
        //printf("Checking if task %d fits\n", approved_tasks[i]->id);
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

int response_time(Task *task, Task **hp_set, int num_tasks)
{
    //printf("NORMAL Task %d (T=%d, C=%d): WCRT=?\n", task->id, task->period, task->duration);
    int wcrt_guess = task->duration;
    int wcrt;

    int base = task->duration;
    int deadline = task->deadline - task->max_jitter;

    while (1)
    {
        wcrt = wcrt_guess;
        wcrt_guess = base;
        //printf("R = %d", base);
        for (int i = 0; i < num_tasks; i++)
        {
            Task *t = hp_set[i];
            wcrt_guess += ceil((float)(wcrt + t->max_jitter) / (float)t->period) * t->duration;
            //printf(" + %d", (int)ceil((float)(wcrt + t->max_jitter) / (float)t->period) * t->duration);
            //printf(" + ceil((%d + %d) / %d) * %d", wcrt, t->max_jitter, t->period, t->duration);
        }

        //printf(" = %d\n", wcrt_guess);

        if (wcrt_guess > deadline)
        {
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

int response_time_migration(Task *task, Task **hp_set, int num_tasks)
{
    //printf("MIGATION Task %d (T=%d, C=%d): WCRT=?\n", task->id, task->period, task->duration);
    int wcrt_guess = task->remaining_execution_time;
    int wcrt;

    int base = task->remaining_execution_time;
    int relative_task_time = task->period - task->time_until_next_period;
    if (!is_ready(task) && base > 0)
        base += task->max_jitter - relative_task_time;

    int deadline = task->remaining_deadline;

    while (1)
    {
        wcrt = wcrt_guess;
        wcrt_guess = base;
        //printf("R = %d", base);
        for (int i = 0; i < num_tasks; i++)
        {
            Task *t = hp_set[i];
            wcrt_guess += t->remaining_execution_time + ceil((float)(wcrt - t->time_until_next_period) / (float)t->period) * t->duration;
    
            //printf(" + %d + ceil((%d - %d) / %d) * %d", t->remaining_execution_time, wcrt, t->time_until_next_period, t->period, t->duration);
            //printf(" + %d", t->remaining_execution_time + (int)ceil((float)(wcrt - t->time_until_next_period) / (float)t->period) * t->duration);
        }

        //printf(" = %d\n", wcrt_guess);

        if (wcrt_guess > deadline)
        {
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

int response_time_custom(Task *task, Task **hp_set, int num_tasks, int migration) {
    if (migration)
        return response_time_migration(task, hp_set, num_tasks);
    else
        return response_time(task, hp_set, num_tasks);
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

    if (compare) // If scheduler has strict rule, make sure to follow it
        prioritize(tasks_copy, num_tasks + 1, compare);
    else // If scheduler has no strict rule, use OPA
        OPA_random(tasks_copy, num_tasks + 1);

    int feasible = RTA_custom(tasks_copy, num_tasks + 1, NULL, migration);
    
    free(tasks_copy);
    return feasible;
}

int RTA_test_with_migration(TaskGroup *group, Task *task, int (*compare)(const void *, const void *))
{
    return RTA_test_with_custom(group, task, compare, 1);
}

int RTA_test_with(TaskGroup *group, Task *task, int (*compare)(const void *, const void *))
{
    return RTA_test_with_custom(group, task, compare, 0);
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
