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

// Recreating taskshuffler results
int main(void)
{
    // random seed using current time
    srand(time(NULL) ^ clock());

    int m = 1;

    int numOfTasks[6] = {5, 7, 9, 11, 13, 15};

    double utilgroups[20];
    for (int i = 0; i < 10; i++)
    {
        utilgroups[2 * i] = 0.02 + 0.1 * i;
        utilgroups[2 * i + 1] = 0.08 + 0.1 * i;
    }

    int hyper_period = 3000;

    for (int i = 0; i < 1; i++)
    {
        for (int n_idx = 0; n_idx < 6; n_idx++)
        {
            for (int u = 0; u < 10; u++)
            {

                int n = numOfTasks[n_idx];
                double U_low = utilgroups[2 * u];
                double U_high = utilgroups[2 * u + 1];
                double U;
                double actual_U;

                Task **tasks;
                while (1)
                {
                    U = U_low + (U_high - U_low) * rand() / (RAND_MAX + 1.0);
                    Task **tasks_attempt = generate_task_set(n, U, hyper_period, 1, 50);

                    actual_U = 0;
                    for (int j = 0; j < n; j++)
                    {
                        actual_U += tasks_attempt[j]->utilization;
                    }

                    if (RTA(tasks_attempt, n, &SM))
                    {
                        tasks = tasks_attempt;
                        break;
                    }

                    else
                    {
                        free_tasks(tasks_attempt, n);
                    }
                }

                Processor *processor = init_processor_custom(m, &init_scheduler_ts);

                processor->log_attack_data = 0;
                processor->log_timeslot_data = 1;
                processor->analyze = &analyze_simulation;

                load_tasks(processor, tasks, n, &ff);

                double result[3];
                run(processor, hyper_period * 10000, result);
                
                printf("U=%.2f,", U);
                printf("n=%d,", n);
                printf("E=%.3f,", result[0]);
                printf("\n");

                free_processor(processor);
                free_tasks(tasks, n);
            }
        }
    }
}
