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

// Litmus test
double sim(int n, int m, Task **tasks, int *allocation)
{
    int hyper_period = 3000;

    Processor *processor = init_processor_custom(m, &init_scheduler_ts);

    processor->log_attack_data = 1;
    processor->log_timeslot_data = 0;
    processor->analyze = &analyze_simulation;

    int load_was_successful = load_tasks_from_allocation(processor, tasks, n, allocation);

    if (!load_was_successful)
    {
        free_processor(processor);
        return 0;
    }

    // print task partition
    /* for (int i = 0; i < m; i++)
    {
        TaskGroup *group = processor->tasks->task_groups[i];
        printf("Core %d: ", i);
        printf("U=%.2f - ", group->utilization);
        for (int j = 0; j < group->num_tasks; j++)
        {
            printf("%d\t", group->tasks[j]->id);
        }

        printf("\n");
    } */

    double result;
    run(processor, hyper_period * 1000, &result);

    free_processor(processor);

    return result;
}

// Example usage
int main(void)
{
    // printf("Starting...\n");
    // srand(time(NULL) ^ clock());
    srand(1);
    int m = 1; // Number of bins
    double U = 0.5;
    for (int n = 5; n < 25; n += 1)
    {
        int hyper_period = 3000;

        Task **tasks = generate_task_set(n, U * m, hyper_period, 1, 50);
        prioritize(tasks, n, &RM);
        int untrusted_idx = rand() % n; // Randomly select an untrusted task
        tasks[untrusted_idx]->trusted = 0;

        /* double actual_U = 0;
        for (int i = 0; i < n; i++)
        {
            actual_U += tasks[i]->utilization;
        } */

        // compare with WF-DU
        Processor *processor = init_processor_custom(m, &init_scheduler_ts);

        processor->log_attack_data = 1;
        processor->log_timeslot_data = 0;
        processor->analyze = &analyze_simulation;

        // int load_was_successful = load_tasks_with_algorithm_argument(processor, tasks, n, &ff_50percent_custom, 0.50);
        int load_was_successful = load_tasks(processor, tasks, n, &wf);

        if (!load_was_successful)
        {
            free_processor(processor);
            free_tasks(tasks, n);
            continue;
        }

        // print task partition
        /* for (int i = 0; i < m; i++)
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

        double result;

        run(processor, hyper_period * 1000, &result);

        free_processor(processor);

        // printf("CP1=%.3f\n", ff);

        free_tasks(tasks, n);

        printf("n=%d,", n);
        printf("ANT=%.3f,", result);
        printf("\n");
    }

    return 0;
}
