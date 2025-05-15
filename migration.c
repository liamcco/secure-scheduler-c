#include <stdlib.h>
#include <stdio.h>

#include "migration.h"
#include "feasibility.h"
#include "priority.h"
#include "processor.h"
#include "debug.h"

void attempt_random_migration(Processor *processor) {
    Partition *tasks = processor->tasks;
    // Pick random core + task
    int core = rand() % tasks->num_groups;
    if (tasks->task_groups[core]->num_tasks == 0) {
        return;
    }

    TaskGroup *group = tasks->task_groups[core];
    int task_idx = rand() % group->num_tasks;
    Task *task = group->tasks[task_idx];

    // Pick random OTHER core to move the task to
    int new_core = rand() % tasks->num_groups;
    if (new_core == core) {
        return;
    }

    TaskGroup *new_group = tasks->task_groups[new_core];
    int (*new_scheduler_comp)(const void *, const void *) = processor->cores[new_core]->scheduler->compare;
    int feasible = RTA_test_with(new_group, task, new_scheduler_comp);
    int migration_is_possible = RTA_test_with_migration(new_group, task, new_scheduler_comp);

    if (feasible && migration_is_possible) {
        // Task can switch. Make the switch
        // 1. Move tasks
        new_group->tasks[new_group->num_tasks] = group->tasks[task_idx];
        new_group->num_tasks++;

        group->tasks[task_idx] = NULL;
        for (int i = task_idx; i < group->num_tasks; i++) {
            group->tasks[i] = group->tasks[i+1];
        }
        group->num_tasks--;

        if (processor->debug)
            debug_partition(processor->tasks);

        load_tasks_core_mid(processor->cores[core], group);
        load_tasks_core_mid(processor->cores[new_core], new_group);
    }
}

void random_migration(Processor *processor) {
    if (processor->tasks->num_groups == 1)
        return;

    attempt_random_migration(processor);
}
