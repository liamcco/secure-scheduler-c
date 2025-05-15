#include <stdlib.h>
#include <stdio.h>

#include "processor.h"
#include "partition_algorithms.h"
#include "simulation.h"
#include "priority.h"
#include "simulation.h"
#include "migration.h"
#include "reprioritze.h"

Processor *init_processor(int m)
{
    return init_processor_custom(m, &init_scheduler_fp);
}

Processor *init_processor_custom(int m, Scheduler *(*init_scheduler)(void))
{
    Processor *processor = (Processor *)malloc(sizeof(Processor));

    processor->m = m;
    processor->cores = (Core **)malloc(m * sizeof(Core *));
    for (int i = 0; i < processor->m; i++)
    {
        processor->cores[i] = init_core(i, init_scheduler);
    }

    processor->ready_tasks = (Task **)malloc(m * sizeof(Task *));

    processor->tasks = NULL;
    processor->all_tasks = NULL;

    processor->migration = 0;
    processor->reprioritize = 0;

    processor->log_schedule = 0;
    processor->log_attack_data = 0;
    processor->log_timeslot_data = 0;
    processor->analyze = NULL;

    processor->debug = 0;
    processor->t_mitigation = 0;

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
        load_tasks_core(processor->cores[i], group);
    }

    return 1;
}

int load_tasks_with_algorithm_argument(
    Processor *processor,
    Task **tasks, int num_tasks,
    Partition *(partition_algorithm)(Task **, int, int, double), double fraction)
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
    Partition *partitioned_tasks = partition_algorithm(tasks, num_tasks, processor->m, fraction);

    if (!check_partition(partitioned_tasks, num_tasks))
    {
        return 0;
    }

    processor->tasks = partitioned_tasks;

    // Load the tasks into the processor
    for (int i = 0; i < processor->m; i++)
    {
        TaskGroup *group = processor->tasks->task_groups[i];
        load_tasks_core(processor->cores[i], group);
    }

    return 1;
}

int load_tasks_from_allocation(
    Processor *processor,
    Task **tasks, int num_tasks,
    int *allocation)
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
    Partition *partitioned_tasks = partition_from_allocation(tasks, num_tasks, processor->m, allocation);
    if (!check_partition(partitioned_tasks, num_tasks))
    {
        return 0;
    }

    processor->tasks = partitioned_tasks;

    // Load the tasks into the processor
    for (int i = 0; i < processor->m; i++)
    {
        TaskGroup *group = processor->tasks->task_groups[i];
        load_tasks_core(processor->cores[i], group);
    }

    return 1;
}

int run(Processor *processor, int hyperperiod, int num_hyperperiods, double *result)
{
    setup_simulation(processor, hyperperiod, num_hyperperiods);

    int time = hyperperiod * num_hyperperiods;

    for (int t = 0; t < time; t++)
    {        

        prepare_tasks_processor(processor);
        log_execution(processor, t);
        time_step_processor(processor);

        //if (t % hyperperiod == hyperperiod - 1) {
        if (t % processor->t_mitigation == processor->t_mitigation - 1) {
            if (processor->debug)
                printf("------------------------NEW HYPER PERIOD (t = %d) -------------------------\n", t);
                
            if (processor->migration)
                random_migration(processor);
            
            if (processor->reprioritize) {
                reprioritze(processor);
            }
        }
    }

    if (processor->log_attack_data) {
        check_new_tasks_for_attacks(processor->all_tasks, processor->simulation->attack_data);
        check_new_tasks_for_attacks(processor->all_tasks, processor->simulation->attack_data_h);
    }

    if (processor->analyze)
        processor->analyze(processor, result);

    teardown_simulation(processor);

    return 1;
}
