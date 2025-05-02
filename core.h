#ifndef CORE_H
#define CORE_H

#include "task.h"
#include "scheduler.h"
#include "partition_algorithms.h"

typedef struct Core
{
    int core_id;

    TaskGroup *group;

    Task *task_to_execute;
    Scheduler *scheduler;
} Core;

Core *init_core(int core_id, Scheduler *(*init_scheduler)(void));
void free_core(Core *core);
Task *load_next_task(Core *core);
void execute_core(Core *core);
void time_step_core(Core *core);
void load_tasks_core(Core *core, TaskGroup *group);
void load_tasks_core_mid(Core *core, TaskGroup *group);

#endif // CORE_H
