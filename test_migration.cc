#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "task.h"
#include "processor.h"
#include "partition_algorithms.h"
#include "taskset.h"
#include "experiments.h"
#include "feasibility.h"
#include "priority.h"
#include "opa.h"

void try_simulation(Task **tasks, int n, double* result, int migration)
{
    Processor *processor = init_processor_custom(2, &init_scheduler_ts); // Unicore

    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;
    processor->migration = migration;
    processor->analyze = &analyze_simulation; // <- This is the function that calculates the result (experiments.c)

    int load_successful = load_tasks(processor, tasks, n, &ff);
    if (!load_successful)
    {
        free_processor(processor);
        return;
    }

    // Reset tasks if task set is to be reused
    for (int i = 0; i < n; i++)
    {
        reset(tasks[i]);
    }

    run(processor, 3000, 1000, result);

    free_processor(processor);
    return;
}

// Recreating taskshuffler results
int main(void)
{
    // random seed using current time
    srand(time(NULL) ^ clock());
    srand(0);

    int hyper_period = 3000;
    int n = 10;

    double U = 0.5;

    Task **tasks = generate_task_set(n, 2*U, hyper_period, 1, 50);
    prioritize(tasks, n, &RM);

    for (int j = 0; j < n; j++)
    {
        tasks[j]->id = j + 1; // Assign task IDs
    }

    double rm[8];
    for (int i = 0; i < 8; i++)
    {
        rm[i] = 0;
    }
    try_simulation(tasks, n, rm, 0); // Run simulation with RM
    printf("RM without miration: %.3f\n", rm[6]);

    for (int i = 0; i < 8; i++)
    {
        rm[i] = 0;
    }
    try_simulation(tasks, n, rm, 1);
    printf("RM with migration: %.3f\n", rm[6]);

    free_tasks(tasks, n);
    
    return 0;
}
