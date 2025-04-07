#include <stdlib.h>
#include <stdio.h>

#include "core.h"

Core *init_core(int core_id, Scheduler *(*init_scheduler)(void))
{
    Core *core = (Core *)malloc(sizeof(Core));
    core->core_id = core_id;

    Scheduler *scheduler = init_scheduler();
    scheduler->idle_task->id = -core_id;
    core->scheduler = scheduler;

    return core;
}

void free_core(Core *core)
{
    free_scheduler(core->scheduler);
    free(core);
}

void load_tasks_core(Core *core, TaskGroup *group)
{
    core->group = group;

    for (int i = 0; i < group->num_tasks; i++)
    {
        group->tasks[i]->c_id = core->core_id;
    }

    load_tasks_scheduler(core->scheduler, group->tasks, group->num_tasks);
}
Task *load_next_task(Core *core)
{
    Task *selected_task = schedule_task(core->scheduler);
    core->task_to_execute = selected_task;
    return selected_task;
}

void execute_core(Core *core)
{

    int did_complete = execute(core->task_to_execute);
    if (did_complete)
    {
        task_completed(core->scheduler, core->task_to_execute);
    }
}

void debug(Core *core)
{
    printf("Core %d:\n", core->core_id);
    for (int j = 0; j < core->group->num_tasks; j++)
    {
        Task *task = core->group->tasks[j];
        printf("Task %d: T=%d C=%d F=%d\n", task->id, task->period, task->duration, task->max_jitter);
    }

    printf("\n");
    exit(EXIT_FAILURE);
}

void time_step_core(Core *core)
{
    time_step_scheduler(core->scheduler);

    execute_core(core);
    for (int i = 0; i < core->group->num_tasks; i++)
    {
        if (time_step_task(core->group->tasks[i]))
        {
            task_arrived(core->scheduler, core->group->tasks[i]);
        }
    }
}
