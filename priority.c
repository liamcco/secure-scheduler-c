#include <stdio.h>
#include <stdlib.h>

#include "priority.h"

void prioritize(Task **tasks, int num_tasks, int (*compare)(const void *, const void *))
{
    // Sort tasks by period
    qsort(tasks, num_tasks, sizeof(Task *), compare);
}

int RM(const void *elem1, const void *elem2)
{
    Task *task1 = *(Task **)elem1;
    Task *task2 = *(Task **)elem2;

    int f_period = task1->period;
    int s_period = task2->period;
    return f_period - s_period;
}

int RRM(const void *elem1, const void *elem2)
{
    return -RM(elem1, elem2);
}

int DM(const void *elem1, const void *elem2)
{
    Task *task1 = *(Task **)elem1;
    Task *task2 = *(Task **)elem2;

    int f_deadline = task1->deadline;
    int s_deadline = task2->deadline;
    return f_deadline - s_deadline;
}

int RDM(const void *elem1, const void *elem2)
{
    return -DM(elem1, elem2);
}

int IU(const void *elem1, const void *elem2)
{
    Task *task1 = *(Task **)elem1;
    Task *task2 = *(Task **)elem2;

    double f_utilization = task1->utilization;
    double s_utilization = task2->utilization;

    if (f_utilization > s_utilization)
    {
        return 1;
    }
    else if (f_utilization < s_utilization)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int DU(const void *elem1, const void *elem2)
{
    return -IU(elem1, elem2);
}

int SM(const void *elem1, const void *elem2)
{
    Task *task1 = *(Task **)elem1;
    Task *task2 = *(Task **)elem2;

    int f_slack = task1->deadline - task1->duration;
    int s_slack = task2->deadline - task2->duration;
    return f_slack - s_slack;
}

int RSM(const void *elem1, const void *elem2)
{
    return -SM(elem1, elem2);
}