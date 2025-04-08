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


int sim_partition(Task **tasks, int n, int m, double *result, Partition *(*partition_algorithm)(Task **, int, int), int (*compare)(const void *, const void *))
{
    // Initialize processor and load tasks
    Processor *processor = init_processor_custom(m, &init_scheduler_ts);
    processor->log_attack_data = 1;
    processor->log_timeslot_data = 0;
    processor->migration = 0;
    processor->analyze = &analyze_simulation;

    // Load tasks according to the given allocation
    prioritize(tasks, n, compare);
    int load_was_successful = load_tasks(processor, tasks, n, partition_algorithm);
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
    int n = 25; // Number of tasks
    int m = 4; // Number of bins

    for (int f = 5; f <= 100; f+=20) {
    double fraction_untrusted = (double)f / 100.0;
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

    printf("U=%.2f,", actual_U);
    printf("f=%.2f,", (double)num_untrusted_tasks / (double)n);

    int success = sim_partition(tasks, n, m, result, &wf, &RM);
    if (success) {
        printf("WF-RM_an=%.3f,", result[0]);
        printf("WF-RM_po=%.3f,", result[1]);
        printf("WF-RM_pi=%.3f,", result[2]);
    }
    success = sim_partition(tasks, n, m, result, &bf, &IU);
    if (success) {
        printf("BF-IU_an=%.3f,", result[0]);
        printf("BF-IU_po=%.3f,", result[1]);
        printf("BF-IU_pi=%.3f,", result[2]);
    }
    success = sim_partition(tasks, n, m, result, &ff, &RM);
    if (success) {
        printf("FF-RM_an=%.3f,", result[1]);
        printf("FF-RM_po=%.3f,", result[2]);
        printf("FF-RM_pi=%.3f,", result[3]);
    }

    free_tasks(tasks, n);
    printf("\n");

    }
}

    return 0;
}
