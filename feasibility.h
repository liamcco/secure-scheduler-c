#ifndef FEASIBILITY_H
#define FEASIBILITY_H

#include "task.h"
#include "partition_algorithms.h"

int RTA(Task **tasks, int num_tasks, int (*compare)(const void *, const void *));
int RTA_migration(Task **tasks, int num_tasks, int (*compare)(const void *, const void *));
int response_time(Task *task, Task **task_set, int num_tasks);
int response_time_migration(Task *task, Task **task_set, int num_tasks);
int RTA_test_with(TaskGroup *group, Task *task, int (*compare)(const void *, const void *));
int RTA_test_with_migration(TaskGroup *group, Task *task, int (*compare)(const void *, const void *));

int LL_test_with(TaskGroup *group, Task *task);

#endif // FEASIBILITY_H
