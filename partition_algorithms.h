#ifndef PARTITION_ALGORITHMS_H
#define PARTITION_ALGORITHMS_H

#include "task.h"

typedef struct TaskGroup
{
    Task **tasks;
    int num_tasks;
    double utilization;
} TaskGroup;

typedef struct Partition
{
    struct TaskGroup **task_groups;
    int num_groups;
} Partition;

void free_partition(Partition *partition);

int check_partition(Partition *partition, int num_tasks);

Partition *ff(Task **tasks, int num_tasks, int m);
Partition *nf(Task **tasks, int num_tasks, int m);
Partition *bf(Task **tasks, int num_tasks, int m);
Partition *wf(Task **tasks, int num_tasks, int m);
Partition *rndf(Task **tasks, int num_tasks, int m);

#endif // PARTITION_ALGORITHMS_H