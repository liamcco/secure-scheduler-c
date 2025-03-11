#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "task.h"
#include "processor.h"
#include "partition_algorithms.h"
#include "taskset.h"
#include "experiments.h"
#include "feasibility.h"
#include "priority.h"

// Litmus test
double sim(int n, int m, Task **tasks)
{
    int hyper_period = 3000;

    Processor *processor = init_processor_custom(m, &init_scheduler_ts);

    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;
    processor->analyze = &analyze_simulation;

    int load_was_successful = load_tasks(processor, tasks, n, &wf);

    if (!load_was_successful)
    {
        free_processor(processor);
        return 0;
    }

    double result;
    run(processor, hyper_period * 1000, &result);

    free_processor(processor);

    return result;
}

// Example usage
int main(void)
{
    srand(time(NULL) ^ clock());
    int hyper_period = 3000;
    for (int n = 10; n < 20; n++)
    {
        for (int Uidx = 10; Uidx < 650; Uidx += 10)
        {
            double U = Uidx / 100.0;
            Task **tasks = generate_task_set(n, U, 3000, 1, 50);
            prioritize(tasks, n, &DU);

            int minm = 0;
            int bestm = minm;
            double bestresult = 0;

            for (int m = ceil(U); m <= n; m++)
            {
                Processor *processor = init_processor_custom(m, &init_scheduler_ts);

                processor->log_attack_data = 0;
                processor->log_timeslot_data = 1;
                processor->analyze = &analyze_simulation;

                int load_was_successful = load_tasks(processor, tasks, n, &wf);

                if (!load_was_successful)
                {
                    free_processor(processor);
                    continue;
                }

                if (!minm)
                {
                    minm = m;
                }

                double result;
                run(processor, hyper_period * 1000, &result);
                if (result > bestresult)
                {
                    bestresult = result;
                    bestm = m;
                }

                free_processor(processor);
            }

            free_tasks(tasks, n);
            printf("U = %f, minm = %d, bestm = %d, n = %d\n", U, minm, bestm, n);
        }
    }

    return 0;
}
