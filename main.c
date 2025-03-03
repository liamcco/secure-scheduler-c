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

Partition *custom_loader(Task **tasks, int num_tasks, int m)
{
    // prioritize(tasks, num_tasks, &RM);
    return even2(tasks, num_tasks, m, num_tasks);
}

// taskShuffler
int main()
{
    // random seed using current time
    srand(time(NULL) ^ clock());

    int m = 4;

    int numOfTasks[6] = {5, 7, 9, 11, 13, 15};

    double utilgroups[20];
    for (int i = 0; i < 10; i++)
    {
        utilgroups[2 * i] = 0.02 + 0.1 * i;
        utilgroups[2 * i + 1] = 0.08 + 0.1 * i;
    }

    int hyper_period = 3000;

    for (int i = 0; i < 20; i++)
    {
        for (int n_idx = 0; n_idx < 6; n_idx++)
        {

            int n = numOfTasks[n_idx] * m;
            double U = 0.5 * m;

            while (1)
            {
                Task **tasks = generate_task_set(n, U, hyper_period, 1, 50);
                /*  if (RTA(tasks_attempt, n, &RM))
                {
                    tasks = tasks_attempt;
                    break;
                } */
                Processor *processor = init_processor_custom(m, &init_scheduler_ts);

                processor->log_attack_data = 1;
                processor->log_timeslot_data = 0;
                processor->analyze = &analyze_simulation;

                int load_was_successful = load_tasks(processor, tasks, n, &custom_loader);

                if (!load_was_successful)
                {
                    free_processor(processor);
                    free_tasks(tasks, n);
                    continue;
                }

                // print task partition
                for (int i = 0; i < m; i++)
                {
                    TaskGroup *group = processor->tasks->task_groups[i];
                    printf("Core %d:\t", i);
                    for (int j = 0; j < group->num_tasks; j++)
                    {
                        printf("%d\t", group->tasks[j]->id);
                    }
                    printf("U=%.2f", group->utilization);
                    printf("\n");
                }

                int success = run(processor, hyper_period * 1000);

                free_processor(processor);
                free_tasks(tasks, n);

                if (success)
                {
                    break;
                }
            }
        }
    }
}