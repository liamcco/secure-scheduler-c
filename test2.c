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
    processor->log_timeslot_data = 1;
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

    run(processor, hyper_period, 1000, result);

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
    int hyper_period = 3000;
    // random int for id
    

    for (int U = 2; U < 80; U++)
    {
        double u = (double)U / 100;
        Task **tasks = generate_task_set(n, u * m, hyper_period, 1, 50);
        prioritize(tasks, n, &DU);
        // OPA_with_priority(tasks, n, &DU);


            tasks[0]->trusted = 0;

            double actual_U = 0;
            for (int i = 0; i < n; i++)
            {
                actual_U += tasks[i]->utilization;
            }

            double result[8] = {-1, -1, -1, -1, -1, -1, -1, -1};

            sim(n, m, tasks, result);

            if (result[0] == -1)
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

            printf("U=%.2f,", actual_U / (double)m);
            printf("ANT_H=%.3f,", result[0]);
            printf("POS_H=%.3f,", result[1]);
            printf("PIN_H=%.3f,", result[2]);
            printf("ANT_V=%.3f,", result[3]);
            printf("POS_V=%.3f,", result[4]);
            printf("PIN_V=%.3f,", result[5]);
            printf("E_H=%.3f,", result[6]);
            printf("E_V=%.3f,", result[7]);
            printf("\n");

        free_tasks(tasks, n);
    }

    return 0;
}
