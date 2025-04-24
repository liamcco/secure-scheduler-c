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
#include "scheduler.h" 

Scheduler *init_scheduler_risat(void) {
    Scheduler *scheduler = init_scheduler_ts();
    scheduler->risat_budget = 1;
    return scheduler;
}

int sim_partition(Task **tasks, int n, int m, double *result, int risat)
{

    Scheduler *(*init_scheduler)(void);

    if (risat)
    {
        init_scheduler = &init_scheduler_risat;
    }
    else
    {
        init_scheduler = &init_scheduler_ts;
    }

    // Initialize processor and load tasks
    Processor *processor = init_processor_custom(m, init_scheduler);
    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;

    processor->analyze = &analyze_simulation;

    // Load tasks according to the given allocation
    prioritize(tasks, n, &DU);

    int load_was_successful = load_tasks(processor, tasks, n, &wf);
    if (!load_was_successful)
    {
        free_processor(processor);
        return 0;
    }

    for (int i = 0; i < n; i++)
    {
        reset(tasks[i]);
    }

    // Run simulation
    run(processor, 3000, 1000, result);

    free_processor(processor);
    return 1;
}

// Example usage
int main(void)
{
    // printf("Starting...\n");
    srand(time(NULL) ^ clock());
    int n = 25; // Number of tasks
    int m = 4; // Number of bins

    for (int u = 50; u < 81; u++)
    {
    double U = (double)u / 100.0;
    int hyper_period = 3000;

    Task **tasks = generate_task_set(n, U * m, hyper_period, 1, 50);
    double actual_U = 0;
    for (int i = 0; i < n; i++)
    {
        actual_U += tasks[i]->utilization;
    }

    // print task partition
    /*for (int i = 0; i < m; i++)
    {
        TaskGroup *group = processor->tasks->task_groups[i];
        printf("Core %d: ", i);
        printf("U=%.2f - ", group->utilization);
        for (int j = 0; j < group->num_tasks; j++)
        {
            printf("%d (T=%d C=%d U=%.2f)\t", group->tasks[j]->id, group->tasks[j]->period, group->tasks[j]->duration, group->tasks[j]->utilization);
        }
        printf("\n");
    } */

    double result[8];
    for (int i = 0; i < 8; i++)
    {
        result[i] = 0;
    }

    int success = sim_partition(tasks, n, m, result, 0);
    if (!success) {
        continue;
    }
    double normal_h = result[7];
    success = sim_partition(tasks, n, m, result, 1);
    if (!success) {
        continue;
    }
    double risat_h = result[7];

    double result_h = risat_h / normal_h;

    printf("U=%.2f,", actual_U);

    printf("result_v=%.3f,", result_h);

    free_tasks(tasks, n);
    printf("\n");

    }

    return 0;
}
