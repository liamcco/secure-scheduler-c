#include <stdlib.h>
#include <stdio.h>

#include "migration.h"
#include "feasibility.h"
#include "priority.h"
#include "processor.h"

void debug_partition(Partition *tasks) {
    for (int m = 0; m < tasks->num_groups; m++) {
        printf("Core %d: ", m + 1);
        Task **core_tasks = tasks->task_groups[m]->tasks;
        for (int i = 0; i < tasks->task_groups[m]->num_tasks; i++) {
            Task *task = core_tasks[i];
            printf("Task %d (T=%d C=%d U=%.2f) ", task->id, task->period, task->duration, task->utilization);
        }
        printf("\n");
    }
}

void debug_scheduler(Scheduler *scheduler) {
    printf("Scheduler tasks: ");
    for (int i = 0; i < scheduler->num_tasks; i++) {
        Task *task = scheduler->tasks[i];
        printf("Task %d ", task->id);
    }
    printf("\nIdle task c_idx=%d\n", scheduler->idle_task->c_idx);
}

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
    while (new_core == core) {
        new_core = rand() % tasks->num_groups;
    }

    TaskGroup *new_group = tasks->task_groups[new_core];

    // Time this line
    //int migration_is_possible = RTA_test_with(new_group, task, &RM);
    int migration_is_possible = LL_test_with(new_group, task);

    if (migration_is_possible) {
        // Task can switch. Make the switch
        // 1. Move tasks
        new_group->tasks[new_group->num_tasks] = group->tasks[task_idx];
        new_group->num_tasks++;

        group->tasks[task_idx] = NULL;
        for (int i = task_idx; i < group->num_tasks; i++) {
            group->tasks[i] = group->tasks[i+1];
        }
        group->num_tasks--;

        load_tasks_core(processor->cores[core], group);
        load_tasks_core(processor->cores[new_core], new_group);

    }
}

void random_migration(Processor *processor) {
    if (processor->tasks->num_groups == 1)
        return;

    for (int i = 0; i < 20; i++) {
        attempt_random_migration(processor);
    }
}
