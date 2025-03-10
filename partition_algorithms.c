#include <stdlib.h>
#include <stdio.h>

#include "priority.h"
#include "partition_algorithms.h"
#include "feasibility.h"

Partition *init_partition(int num_tasks, int m)
{
    Partition *partition = (Partition *)malloc(sizeof(Partition));
    partition->num_groups = m;
    partition->task_groups = (TaskGroup **)malloc(m * sizeof(TaskGroup *));
    for (int i = 0; i < m; i++)
    {
        TaskGroup *group = (TaskGroup *)malloc(sizeof(TaskGroup));
        group->num_tasks = 0;
        group->tasks = (Task **)malloc(num_tasks * sizeof(Task *));
        group->utilization = 0;
        partition->task_groups[i] = group;
    }
    return partition;
}

void free_partition(Partition *partition)
{
    for (int i = 0; i < partition->num_groups; i++)
    {
        TaskGroup *group = partition->task_groups[i];
        free(group->tasks);
        free(group);
    }
    free(partition->task_groups);
    free(partition);
}

int check_partition(Partition *partition, int num_tasks)
{
    int allocated_tasks = 0;
    for (int i = 0; i < partition->num_groups; i++)
    {
        allocated_tasks += partition->task_groups[i]->num_tasks;
    }

    return allocated_tasks == num_tasks;
}

Partition *ff(Task **tasks, int num_tasks, int m)
{
    Partition *partitioned_tasks = init_partition(num_tasks, m);

    for (int i = 0; i < num_tasks; i++)
    {
        Task *task = tasks[i];
        for (int core = 0; core < m; core++)
        {
            TaskGroup *group = partitioned_tasks->task_groups[core];
            if (RTA_test_with(group, task, &RM))
            {
                group->tasks[group->num_tasks] = task;
                group->num_tasks++;
                break;
            }
        }
    }

    return partitioned_tasks;
}

Partition *nf(Task **tasks, int num_tasks, int m)
{
    Partition *partitioned_tasks = init_partition(num_tasks, m);

    int current_core = 0;
    for (int i = 0; i < num_tasks; i++)
    {
        Task *task = tasks[i];
        for (int j = 0; j < m; j++)
        {
            TaskGroup *group = partitioned_tasks->task_groups[(current_core + j) % m];
            if (RTA_test_with(group, task, &RM))
            {
                group->tasks[group->num_tasks] = task;
                group->num_tasks++;
                current_core = (current_core + j) % m;
                break;
            }
        }
    }

    return partitioned_tasks;
}

Partition *bwf(Task **tasks, int num_tasks, int m, int (*compare)(const void *, const void *))
{
    Partition *partitioned_tasks = init_partition(num_tasks, m);

    for (int i = 0; i < num_tasks; i++)
    {
        Task *task = tasks[i];

        // sort the cores by utilization
        qsort(partitioned_tasks->task_groups, m, sizeof(TaskGroup *), compare);

        for (int core = 0; core < m; core++)
        {
            TaskGroup *group = partitioned_tasks->task_groups[core];
            if (RTA_test_with(group, task, &RM))
            {
                group->tasks[group->num_tasks] = task;
                group->num_tasks++;
                group->utilization += task->utilization;
                break;
            }
        }
    }

    return partitioned_tasks;
}

int compare_task_groups_WF(const void *a, const void *b)
{
    TaskGroup *group_a = *(TaskGroup **)a;
    TaskGroup *group_b = *(TaskGroup **)b;

    if (group_a->utilization < group_b->utilization)
    {
        return -1;
    }
    else if (group_a->utilization > group_b->utilization)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int compare_task_groups_BF(const void *a, const void *b)
{
    return -compare_task_groups_WF(a, b);
}

Partition *bf(Task **tasks, int num_tasks, int m)
{
    return bwf(tasks, num_tasks, m, compare_task_groups_BF);
}

Partition *wf(Task **tasks, int num_tasks, int m)
{
    return bwf(tasks, num_tasks, m, compare_task_groups_WF);
}

void shuffle_task_groups(TaskGroup **task_groups, size_t n)
{
    for (size_t i = n - 1; i > 0; i--)
    {
        size_t j = rand() % (i + 1); // Pick a random index from 0 to i

        // Swap task_groups[i] and task_groups[j]
        TaskGroup *temp = task_groups[i];
        task_groups[i] = task_groups[j];
        task_groups[j] = temp;
    }
}

Partition *rndf(Task **tasks, int num_tasks, int m)
{
    Partition *partitioned_tasks = init_partition(num_tasks, m);

    for (int i = 0; i < num_tasks; i++)
    {
        Task *task = tasks[i];

        // sort the cores by utilization
        shuffle_task_groups(partitioned_tasks->task_groups, m);

        for (int core = 0; core < m; core++)
        {
            TaskGroup *group = partitioned_tasks->task_groups[core];
            if (RTA_test_with(group, task, &RM))
            {
                group->tasks[group->num_tasks] = task;
                group->num_tasks++;
                break;
            }
        }
    }

    return partitioned_tasks;
}

int compare_task_groups_num_tasks(const void *a, const void *b)
{
    TaskGroup *group_a = *(TaskGroup **)a;
    TaskGroup *group_b = *(TaskGroup **)b;

    return group_a->num_tasks - group_b->num_tasks;
}

Partition *even(Task **tasks, int num_tasks, int m)
{
    // Distribute high util tasks
    Partition *partitioned_tasks = init_partition(num_tasks, m);

    // Split the tasks set into m/m+1 and 1/m+1 utilizations
    double total_utilization = 0;
    for (int i = 0; i < num_tasks; i++)
    {
        Task *task = tasks[i];
        total_utilization += task->utilization;
    }

    double high_utilization_limit = total_utilization * m / (m + 1);
    double total_utilization_low_high = 0;

    prioritize(tasks, num_tasks, &DU);

    int first_low_util_task_index = 0;
    for (int i = 0; i < num_tasks; i++)
    {
        Task *task = tasks[i];
        total_utilization_low_high += task->utilization;
        if (total_utilization_low_high > high_utilization_limit)
        {
            first_low_util_task_index = i + 1;
            break;
        }
    }

    for (int i = 0; i < first_low_util_task_index; i++)
    {
        Task *task = tasks[i];

        // sort the cores by utilization
        qsort(partitioned_tasks->task_groups, m, sizeof(TaskGroup *), compare_task_groups_WF);

        for (int core = 0; core < m; core++)
        {
            TaskGroup *group = partitioned_tasks->task_groups[core];
            if (RTA_test_with(group, task, &RM))
            {
                group->tasks[group->num_tasks] = task;
                group->num_tasks++;
                group->utilization += task->utilization;
                break;
            }
        }
    }

    // Distribute low util tasks to cores with lowest amount of cores
    for (int i = first_low_util_task_index; i < num_tasks; i++)
    {
        Task *task = tasks[i];

        // sort the cores by number of tasks
        qsort(partitioned_tasks->task_groups, m, sizeof(TaskGroup *), compare_task_groups_num_tasks);

        for (int core = 0; core < m; core++)
        {
            TaskGroup *group = partitioned_tasks->task_groups[core];
            if (RTA_test_with(group, task, &RM))
            {
                group->tasks[group->num_tasks] = task;
                group->num_tasks++;
                group->utilization += task->utilization;
                break;
            }
        }
    }

    return partitioned_tasks;
}

Partition *even2(Task **tasks, int num_tasks, int m, int limit)
{

    // Distribute high util tasks
    Partition *partitioned_tasks = init_partition(num_tasks, m);
    Partition *temp_partitioned_tasks = init_partition(num_tasks, m);

    prioritize(tasks, num_tasks, &DU);

    for (int j = -1; j < limit; j++)
    {

        int num_low_util_tasks = j;
        int first_low_util_task_index = num_tasks - 1 - num_low_util_tasks;

        for (int i = 0; i < first_low_util_task_index; i++)
        {
            Task *task = tasks[i];

            // sort the cores by utilization
            qsort(temp_partitioned_tasks->task_groups, m, sizeof(TaskGroup *), compare_task_groups_WF);

            for (int core = 0; core < m; core++)
            {
                TaskGroup *group = temp_partitioned_tasks->task_groups[core];
                if (RTA_test_with(group, task, &RM))
                {
                    group->tasks[group->num_tasks] = task;
                    group->num_tasks++;
                    group->utilization += task->utilization;
                    break;
                }
            }
        }

        // Distribute low util tasks to cores with lowest amount of cores
        for (int i = first_low_util_task_index; i < num_tasks; i++)
        {
            Task *task = tasks[i];

            // sort the cores by number of tasks
            qsort(temp_partitioned_tasks->task_groups, m, sizeof(TaskGroup *), compare_task_groups_num_tasks);

            for (int core = 0; core < m; core++)
            {
                TaskGroup *group = temp_partitioned_tasks->task_groups[core];
                if (RTA_test_with(group, task, &RM))
                {
                    group->tasks[group->num_tasks] = task;
                    group->num_tasks++;
                    group->utilization += task->utilization;
                    break;
                }
            }
        }
        if (!check_partition(temp_partitioned_tasks, num_tasks))
        {
            free_partition(temp_partitioned_tasks);
            return partitioned_tasks;
        }

        partitioned_tasks = temp_partitioned_tasks;
        temp_partitioned_tasks = init_partition(num_tasks, m);
    }

    if (temp_partitioned_tasks)
        free_partition(temp_partitioned_tasks);

    return partitioned_tasks;
}

Partition *partition_from_allocation(Task **tasks, int num_tasks, int m, int *allocation)
{
    // Create partition with m groups
    Partition *partition = init_partition(num_tasks, m);

    // Load tasks into partition according to allocation
    for (int i = 0; i < num_tasks; i++)
    {
        Task *task = tasks[i];
        int core = allocation[i] - 1; // -1 for 0-based index

        TaskGroup *group = partition->task_groups[core];
        if (RTA_test_with(group, task, &RM))
        {
            group->tasks[group->num_tasks] = task;
            group->num_tasks++;
            group->utilization += task->utilization;
        }
    }

    return partition;
}