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
    srand(time(NULL) ^ clock());
    int m = 1; // Number of bins
    int n = 10;
    for (int U = 5; U < 85; U += 10)
    {
        double u = (double)U / 100;
        for (int k = 1; k < n; k++)
        {
            int hyper_period = 3000;

            Task **tasks = generate_task_set(n, u * m, hyper_period, 1, 50);
            prioritize(tasks, n, &RM);

            for (int i = n; i > n - k; i--)
            {
                tasks[i - 1]->trusted = 0;
            }

            double actual_U = 0;
            for (int i = 0; i < n; i++)
            {
                actual_U += tasks[i]->utilization;
            }

            // compare with WF-DU
            Processor *processor = init_processor_custom(m, &init_scheduler_fp);

            processor->log_attack_data = 1;
            processor->log_timeslot_data = 0;
            processor->analyze = &analyze_simulation;

            // int load_was_successful = load_tasks_with_algorithm_argument(processor, tasks, n, &ff_50percent_custom, 0.50);
            int load_was_successful = load_tasks(processor, tasks, n, &ff);

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

            double result[3];

            run(processor, hyper_period * 1000, result);

            free_processor(processor);

            // printf("CP1=%.3f\n", ff);

            free_tasks(tasks, n);

            printf("k=%d,", k);
            printf("U=%.3f,", actual_U);
            printf("ANT=%.3f,", result[0]);
            printf("POST=%.3f,", result[1]);
            printf("PINCH=%.3f\n", result[2]);

            printf("\n");
        }
    }

    return 0;
}
