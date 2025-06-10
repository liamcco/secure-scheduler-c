#include <stdio.h>
#include "debug.h"

void debug_partition(Partition *tasks) {
    for (int m = 0; m < tasks->num_groups; m++) {
        printf("\nCore %d:\n", m + 1);
        Task **core_tasks = tasks->task_groups[m]->tasks;
        for (int i = 0; i < tasks->task_groups[m]->num_tasks; i++) {
            Task *task = core_tasks[i];
            printf("Task %d (T=(%d of %d) C=(%d of %d) V=(%d of %d))\n", task->id, task->period - task->time_until_next_period, task->period, task->duration - task->remaining_execution_time, task->duration, task->remaining_inversion_budget, task->maximum_inversion_budget);
        }
        printf("\n");
    }
}

void debug_scheduler(Scheduler *scheduler) {
    printf("Scheduler tasks: ");
    for (int i = 0; i < scheduler->num_tasks; i++) {
        Task *task = scheduler->tasks[i];
        printf("Task %d (T=(%d of %d) C=(%d of %d) V=(%d of %d) Ready=%d)\n", task->id, task->period - task->time_until_next_period, task->period, task->duration - task->remaining_execution_time, task->duration, task->remaining_inversion_budget, task->maximum_inversion_budget, is_ready(task));
    }
    // printf("\nIdle task c_idx=%d\n", scheduler->idle_task->c_idx);
    printf("to_schedule=%d\n", scheduler->to_schedule);

}
