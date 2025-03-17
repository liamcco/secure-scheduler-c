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

Partition *init_partition(int num_tasks, int m);
void free_partition(Partition *partition);

int check_partition(Partition *partition, int num_tasks);

Partition *ff(Task **tasks, int num_tasks, int m);
Partition *ff_custom(Task **tasks, int num_tasks, int m, int (*compare)(const void *, const void *));
Partition *ff_nosort(Task **tasks, int num_tasks, int m);
Partition *ff_50percent_custom(Task **tasks, int num_tasks, int m);
Partition *nf(Task **tasks, int num_tasks, int m);
Partition *bf(Task **tasks, int num_tasks, int m);
Partition *wf(Task **tasks, int num_tasks, int m);
Partition *rndf(Task **tasks, int num_tasks, int m);
Partition *even(Task **tasks, int num_tasks, int m);
Partition *even2(Task **tasks, int num_tasks, int m, int limit);
Partition *partition_from_allocation(Task **tasks, int num_tasks, int m, int *allocation);

#endif // PARTITION_ALGORITHMS_H
