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

Scheduler *init_scheduler_push(void) {
    Scheduler *scheduler = init_scheduler_ts();
    scheduler->push_back = 1;
    return scheduler;
}

Scheduler *init_scheduler_risat(void) {
    Scheduler *scheduler = init_scheduler_ts();
    scheduler->risat_budget= 1;
    return scheduler;
}

Scheduler *init_scheduler_risat_push(void) {
    Scheduler *scheduler = init_scheduler_ts();
    scheduler->push_back = 1;
    scheduler->risat_budget = 1;
    return scheduler;
}

int sim_partition(Task **tasks, int n, double *result, int push, int risat, int reprioritize)
{

    Scheduler *(*init_scheduler)(void);

    if (push)
    {
        init_scheduler = &init_scheduler_push;
    }
    else if (risat)
    {
        init_scheduler = &init_scheduler_risat;
    }
    else if (risat && push)
    {
        init_scheduler = &init_scheduler_risat_push;
    }
    else 
    {
        init_scheduler = &init_scheduler_ts;
    }
    

    // Initialize processor and load tasks
    Processor *processor = init_processor_custom(1, init_scheduler);
    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;
    processor->reprioritize = reprioritize;

    processor->analyze = &analyze_simulation;

    prioritize(tasks, n, &DU);

    int load_was_successful = load_tasks(processor, tasks, n, &wfminm2);
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
    //int n = 10; // Number of tasks
    int m = 1; // Number of bins


    for (int i = 0; i < 5; i++)
    {
    for (int n = 5; n <= 13; n += 2) {
    for (int u = 2; u < 81; u++)
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

    // push, risat, reprioritze

    int success = sim_partition(tasks, n, result, 0, 0, 0);
    if (!success) {
        continue;
    }
    double normal = result[6];

    success = sim_partition(tasks, n, result, 0, 0, 1);
    if (!success) {
        continue;
    }
    double rep = result[6] / normal;

    success = sim_partition(tasks, n, result, 0, 1, 0);
    if (!success) {
        continue;
    }
    double risat = result[6] / normal;

    success = sim_partition(tasks, n, result, 1, 0, 0);
    if (!success) {
        continue;
    }
    double push = result[6] / normal;

    success = sim_partition(tasks, n, result, 1, 1, 0);
    if (!success) {
        continue;
    }
    double push_risat = result[6] / normal;

    success = sim_partition(tasks, n, result, 0, 1, 1);
    if (!success) {
        continue;
    }
    double rep_risat = result[6] / normal;

    success = sim_partition(tasks, n, result, 1, 0, 1);
    if (!success) {
        continue;
    }
    double rep_push = result[6] / normal;

    success = sim_partition(tasks, n, result, 1, 1, 1);
    if (!success) {
        continue;
    }
    double push_risat_rep = result[6] / normal;

    printf("U=%.2f,n=%d,", actual_U, n);

    printf("rep=%.3f,", rep);
    printf("risat=%.3f,", risat);
    printf("push=%.3f,", push);
    printf("push_risat=%.3f,", push_risat);
    printf("rep_risat=%.3f,", rep_risat);
    printf("rep_push=%.3f,", rep_push);
    printf("push_risat_rep=%.3f,", push_risat_rep);

    free_tasks(tasks, n);
    printf("\n");

    }
    }
    }

    return 0;
}
