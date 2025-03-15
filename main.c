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
    Scheduler *scheduler = init_scheduler_ts_custom(NULL);
    return scheduler;
}

double try_simulation(Task **tasks, int n)
{
    Processor *processor = init_processor_custom(1, &init_scheduler_nosort);

    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;
    processor->analyze = &analyze_simulation;

    int load_successful = load_tasks(processor, tasks, n, &ff_nosort);
    if (!load_successful)
    {
        return 0;
    }

    double result = 0;
    run(processor, 3000 * 1000, &result);

    free_processor(processor);

    for (int i = 0; i < n; i++)
    {
        reset(tasks[i]);
    }

    return result;
}

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Recursive function to generate permutations
void generate_permutations(int arr[], int start, int n, Task **tasks, double results[], int *current, int best[], double *best_res, int debug)
{
    if (start == n)
    {
        // Try running simulation with this allocation
        Task **tasks_copy = (Task **)malloc(sizeof(Task *) * n);
        for (int i = 0; i < n; i++)
        {
            tasks_copy[i] = tasks[arr[i]];
        }
        double result = try_simulation(tasks_copy, n);
        if (!result)
        {
            free(tasks_copy);
            return;
        }

        results[*current] = result;
        *current = *current + 1;

        if (result > *best_res)
        {
            *best_res = result;
            for (int i = 0; i < n; i++)
            {
                best[i] = arr[i];
            }
        }

        if (debug)
        {
            printf("But feasible with:\n");
            for (int i = 0; i < n; i++)
            {
                Task *task = tasks_copy[i];
                printf("%d: Task T=%d, C=%d\n", i, task->period, task->duration);
            }
            printf("Result: %f\n\n", result);
            exit(1);
        }
        free(tasks_copy);

        return;
    }

    for (int i = start; i < n; i++)
    {
        // Swap to fix one element at the current position
        swap(&arr[start], &arr[i]);

        // Recur for the next position
        generate_permutations(arr, start + 1, n, tasks, results, current, best, best_res, debug);

        // Backtrack (restore original array)
        swap(&arr[start], &arr[i]);
    }
}

// Comparison function for qsort
int compare(const void *a, const void *b)
{
    double diff = *(double *)a - *(double *)b;
    return (diff > 0) - (diff < 0); // Returns -1, 0, or 1
}

// Recreating taskshuffler results
int main(void)
{
    // random seed using current time
    srand(time(NULL) ^ clock());

    double utilgroups[20];
    for (int i = 0; i < 10; i++)
    {
        utilgroups[2 * i] = 0.02 + 0.1 * i;
        utilgroups[2 * i + 1] = 0.08 + 0.1 * i;
    }

    int hyper_period = 3000;
    for (int u = 0; u < 10; u++)
    {

        int n = 5;
        double U_low = utilgroups[2 * u];
        double U_high = utilgroups[2 * u + 1];
        double U;
        double actual_U;

        U = U_low + (U_high - U_low) * rand() / (RAND_MAX + 1.0);
        Task **tasks = generate_task_set(n, U, hyper_period, 1, 50);

        actual_U = 0;
        for (int j = 0; j < n; j++)
        {
            actual_U += tasks[j]->utilization;
        }

        OPA_with_priority(tasks, n, &SM);

        double rm = try_simulation(tasks, n);
        if (!rm)
        {
            free_tasks(tasks, n);
            continue;
        }

        // Iterate through all possible task priority assignments

        int tasks_arr[n];
        int best[n];
        for (int i = 0; i < n; i++)
        {
            tasks_arr[i] = i;
            best[i] = i;
        }

        int num_permutations; // = factorial(n);
        num_permutations = 1;
        for (int i = 1; i <= n; i++)
        {
            num_permutations *= i;
        }

        int current = 0;
        double results[num_permutations + 1];
        results[current] = rm;
        current++;
        for (int i = 0; i < num_permutations; i++)
        {
            results[i + 1] = 0;
        }

        double best_res = 0;
        generate_permutations(tasks_arr, 0, n, tasks, results, &current, best, &best_res, 0);

        /* printf("Best result: %f\n", best_res);
        for (int i = 0; i < n; i++)
        {
            Task *task = tasks[best[i]];
            int slack = task->deadline - task->duration;
            double utilization = task->utilization;
            printf("%d: Task T=%d\tC=%d\tS=%d\tU=%.3f\n", i, task->period, task->duration, slack, utilization);
        } */

        free_tasks(tasks, n);

        if (current == 1)
        {
            continue;
        }

        // sort results
        qsort(results, current, sizeof(double), &compare);

        double median = results[current / 2];
        double min = results[0];
        double max = results[current - 1];
        double avg = 0;
        for (int i = 0; i < current; i++)
        {
            avg += results[i];
        }
        avg /= current;

        printf("U=%.2f,", actual_U);
        printf("OPASM=%.3f,", rm / max);
        printf("Med=%.3f,", median / max);
        printf("Min=%.3f,", min / max);
        printf("Avg=%.3f", avg / max);
        printf("\n");
    }
}
