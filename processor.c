#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "processor.h"
#include "partition_algorithms.h"
#include "simulation.h"
#include "priority.h"
#include "simulation.h"

Processor *init_processor(int m)
{
    Processor *processor = (Processor *)malloc(sizeof(Processor));

    processor->m = m;
    processor->cores = (Core **)malloc(m * sizeof(Core *));
    for (int i = 0; i < processor->m; i++)
    {
        processor->cores[i] = init_core(i, &init_scheduler_fp);
    }

    processor->ready_tasks = (Task **)malloc(m * sizeof(Task *));

    processor->tasks = NULL;
    processor->all_tasks = NULL;

    processor->log_schedule = 0;
    processor->log_attack_data = 0;
    processor->log_timeslot_data = 0;
    processor->analyze = NULL;

    return processor;
}

Processor *init_processor_custom(int m, Scheduler *(*init_scheduler)())
{
    Processor *processor = (Processor *)malloc(sizeof(Processor));

    processor->m = m;
    processor->cores = (Core **)malloc(m * sizeof(Core *));
    for (int i = 0; i < processor->m; i++)
    {
        processor->cores[i] = init_core(i, init_scheduler);
    }

    processor->ready_tasks = (Task **)malloc(m * sizeof(Task *));
    return processor;
}

void free_processor(Processor *processor)
{
    // Free the cores
    for (int i = 0; i < processor->m; i++)
    {
        free_core(processor->cores[i]);
    }
    free(processor->cores);
    // Free ready tasks
    free(processor->ready_tasks);

    // Free the partition
    if (processor->tasks)
        free_partition(processor->tasks);

    // Free the task group
    if (processor->all_tasks)
        free(processor->all_tasks);

    free(processor);
}

void prepare_tasks_processor(Processor *processor)
{
    for (int i = 0; i < processor->m; i++)
    {
        processor->ready_tasks[i] = load_next_task(processor->cores[i]);
    }
}

void time_step_processor(Processor *processor)
{
    for (int i = 0; i < processor->m; i++)
    {
        time_step_core(processor->cores[i]);
    }
}

int load_tasks(
    Processor *processor,
    Task **tasks, int num_tasks,
    Partition *(partition_algorithm)(Task **, int, int))
{
    processor->all_tasks = (TaskGroup *)malloc(sizeof(TaskGroup));
    processor->all_tasks->tasks = tasks;
    processor->all_tasks->num_tasks = num_tasks;

    for (int i = 0; i < num_tasks; i++)
    {
        tasks[i]->idx = i;
        tasks[i]->id = i + 1;
    }

    // Partition the tasks
    Partition *partitioned_tasks = partition_algorithm(tasks, num_tasks, processor->m);

    if (!check_partition(partitioned_tasks, num_tasks))
    {
        return 0;
    }

    processor->tasks = partitioned_tasks;

    // Load the tasks into the processor
    for (int i = 0; i < processor->m; i++)
    {
        TaskGroup *group = processor->tasks->task_groups[i];
        load_tasks_core(processor->cores[i], group->tasks, group->num_tasks);
    }

    return 1;
}

int run(Processor *processor, int time)
{
    setup_simulation(processor, time);

    for (int t = 0; t < time; t++)
    {
        prepare_tasks_processor(processor);
        log_execution(processor, t);
        time_step_processor(processor);
        // sleep(1);
    }

    if (processor->log_attack_data)
        check_new_tasks_for_attacks(processor);

    if (processor->analyze)
        processor->analyze(processor);

    teardown_simulation(processor);

    return 0;
}
