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

Scheduler *init_scheduler_ts_with_push_back(void)
{
    Scheduler *scheduler = init_scheduler_ts();
    scheduler->push_back = 1;
    return scheduler;
}

Scheduler *init_scheduler_ts_with_risat(void)
{
    Scheduler *scheduler = init_scheduler_ts();
    scheduler->risat_budget = 1;
    return scheduler;
}

Scheduler *init_scheduler_ts_with_push_risat(void)
{
    Scheduler *scheduler = init_scheduler_ts();
    scheduler->push_back = 1;
    scheduler->risat_budget = 1;
    scheduler->adjust_budget = 1;
    return scheduler;
}

int sim_partition(Task **tasks, int n, int m, double *result, int reprioritize, int migration, int push_back, int risat)
{
    Scheduler *(*init_scheduler)(void);

    if (push_back)
    {
        init_scheduler = &init_scheduler_ts_with_push_back;
    }
    else if (risat)
    {
        init_scheduler = &init_scheduler_ts_with_risat;
    }
    else if (risat && push_back)
    {
        init_scheduler = &init_scheduler_ts_with_push_risat;
    }
    else
    {
        init_scheduler = &init_scheduler_ts;
    }

    // Initialize processor and load tasks
    Processor *processor = init_processor_custom(m, init_scheduler);
    processor->log_attack_data = 1;
    processor->log_timeslot_data = 1;
    processor->reprioritize = reprioritize;
    processor->migration = migration;
    processor->analyze = &analyze_simulation;
    processor->t_mitigation = 500;

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
        // print task partition
        /* for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < n; j++)
            {
                printf("%d (T=%d C=%d U=%.2f)\n", tasks[j]->id, tasks[j]->period, tasks[j]->duration, tasks[j]->utilization);
            }
            printf("\n");
        } */

    // Run simulation
    run(processor, 3000, 10, result);
    //printf("Result: %.3f\n", result[6]);
    // Reset tasks

    free_processor(processor);
    return 1;
}

// Example usage
int main(void)
{
    // printf("Starting...\n");
    srand(time(NULL) ^ clock());
    int n = 10; // Number of tasks
    int m = 1; // Number of bins

    for (int f = 4; f <= 20; f+=4) {
    double fraction_untrusted = (double)f / 100.0;
    for (int u = 2; u < 80; u++)
    {
    for (int e = 0; e < 50; e++) {
    double U = (double)u / 100.0;
    int hyper_period = 3000;

    Task **tasks = generate_task_set(n, U * m, hyper_period, 1, 50);

    double actual_U = 0;
    for (int i = 0; i < n; i++)
    {
        actual_U += tasks[i]->utilization;
    }

    // Randomply select n*0.5 indexes in tasks and make them untrusted
    int num_untrusted = (int)(n * fraction_untrusted);
    for (int i = 0; i < num_untrusted; i++)
    {
        int index = rand() % n;
        tasks[index]->trusted = 0;
    }
    // num of untrusted tasks
    int num_untrusted_tasks = 0;
    for (int i = 0; i < n; i++)
    {
        if (tasks[i]->trusted == 0)
        {
            num_untrusted_tasks++;
        }
    }

    double result[8];
    for (int i = 0; i < 8; i++)
    {
        result[i] = 0;
    }

    double nor_an_h = 0;
    double rep_an_h = 0;

    int success = sim_partition(tasks, n, m, result, 0, 0, 0, 0);
    if (!success)
        continue;

    nor_an_h = result[3];
    
    success = sim_partition(tasks, n, m, result, 1, 1, 1, 1);
    if (!success)
        continue;
    
    rep_an_h = result[3];

    double change = rep_an_h / nor_an_h;

    printf("U=%.2f,", actual_U);
    printf("f=%.2f,", (double)num_untrusted_tasks / (double)n);
    printf("score=%.3f -> %.3f   (%.3f)", nor_an_h, rep_an_h, change);

    free_tasks(tasks, n);
    printf("\n");

    }
}
}

    return 0;
}
