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

Scheduler *init_scheduler_nosort(void)
{
    Scheduler *scheduler = init_scheduler_ts_custom(&RM);
    return scheduler;
}

// Litmus test
void sim(int n, int m, Task **tasks, double *result)
{
    int hyper_period = 3000;

    Processor *processor = init_processor_custom(m, &init_scheduler_nosort);

    processor->log_attack_data = 1;
    processor->log_timeslot_data = 0;
    processor->analyze = &analyze_simulation;

    int load_successful = load_tasks(processor, tasks, n, &ff);

    if (!load_successful)
    {
        free_processor(processor);
        return;
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

    run(processor, hyper_period * 1000, result);

    free_processor(processor);

    for (int i = 0; i < n; i++)
    {
        reset(tasks[i]);
    }
}

// Example usage
int main(void)
{
    // printf("Starting...\n");
    srand(time(NULL) ^ clock());
    int m = 4; // Number of bins
    int n = 25;
    for (int U = 5; U < 80; U += 15)
    {
        double u = (double)U / 100;
        for (int k = 1; k < n; k++)
        {
            int hyper_period = 3000;

            Task **tasks = generate_task_set(n, u * m, hyper_period, 1, 50);
            prioritize(tasks, n, &RM);
            // OPA_with_priority(tasks, n, &DU);

            for (int i = n; i > n - k; i--)
            {
                tasks[i - 1]->trusted = 0;
            }

            double actual_U = 0;
            for (int i = 0; i < n; i++)
            {
                actual_U += tasks[i]->utilization;
            }

            double result[3] = {0, 0, 0};

            sim(n, m, tasks, result);
            free_tasks(tasks, n);

            if (result[0] == 0)
            {
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

            // printf("CP1=%.3f\n", ff);

            printf("k=%d,", k);
            printf("U=%.3f,", actual_U / (double)m);
            printf("ANT=%.3f,", result[0]);
            printf("POST=%.3f,", result[1]);
            printf("PINCH=%.3f", result[2]);
            printf("\n");
        }
    }

    return 0;
}
