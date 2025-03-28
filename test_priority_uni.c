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


// Makes sure scheduler does NOT sort tasks
// This is necessary to try custom priority assignments
Scheduler *init_scheduler_nosort(void)
{
    Scheduler *scheduler = init_scheduler_ts_custom(NULL);
    return scheduler;
}

void try_simulation(Task **tasks, int n, double* result)
{
    Processor *processor = init_processor_custom(1, &init_scheduler_nosort); // Unicore

    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;
    processor->horizontal = 0;
    processor->analyze = &analyze_simulation; // <- This is the function that calculates the result (experiments.c)

    int load_successful = load_tasks(processor, tasks, n, &ff_nosort); // <- ff_nosort will fail if the priority assignment is not feasible
                                                                       // (ff assumes RM)
    if (!load_successful)
    {
        return;
    }

    // Reset tasks if task set is to be reused
    for (int i = 0; i < n; i++)
    {
        reset(tasks[i]);
    }

    run(processor, 3000, 1000, result);

    free_processor(processor);
    return;
}

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Recursive function to generate permutations
void generate_permutations(int arr[], int start, int n, Task **tasks, double results[], int *current, int best[], double *best_res)
{
    if (start == n)
    {
        // Try running simulation with this allocation
        Task **tasks_copy = (Task **)malloc(sizeof(Task *) * n);
        for (int i = 0; i < n; i++)
        {
            tasks_copy[i] = tasks[arr[i]];
        }
        double result[3];
        for (int i = 0; i < 3; i++)
        {
            result[i] = 0;
        }
        try_simulation(tasks_copy, n, result);
        if (!result[0])
        {
            free(tasks_copy);
            return;
        }

        results[*current] = result[0];
        *current = *current + 1;

        if (result[0] > *best_res)
        {
            *best_res = result[0];
            for (int i = 0; i < n; i++)
            {
                best[i] = arr[i];
            }
        }

        free(tasks_copy);

        return;
    }

    for (int i = start; i < n; i++)
    {
        // Swap to fix one element at the current position
        swap(&arr[start], &arr[i]);

        // Recur for the next position
        generate_permutations(arr, start + 1, n, tasks, results, current, best, best_res);

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

    int hyper_period = 3000;

    int n = 5;
    double actual_U;
    double U = 0.5;

    Task **tasks = generate_task_set(n, U, hyper_period, 1, 50);

    actual_U = 0;
    for (int j = 0; j < n; j++)
    {
        actual_U += tasks[j]->utilization; // Calculate actual utilization
        tasks[j]->id = j + 1; // Assign task IDs
    }

    int feasible = OPA_with_priority(tasks, n, &RRM); // -> RM
    if (!feasible)
    {
        free_tasks(tasks, n);
        return 0;
    }

    /*
    printf("RM:\n");
    for (int i = 0; i < n; i++)
    {
        Task *task = tasks[i];
        int slack = task->deadline - task->duration;
        double utilization = task->utilization;
        printf("%d: Task %d\tT=%d\tC=%d\tS=%d\tU=%.3f\n", i + 1, task->id, task->period, task->duration, slack, utilization);
    }
    */

    double rm[3];
    for (int i = 0; i < 3; i++)
    {
        rm[i] = 0;
    }
    try_simulation(tasks, n, rm); // Run simulation with RM
    printf("RM: %.3f\n", rm[0]);

    exit(0);

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
    results[current] = rm[0];
    current++;
    for (int i = 0; i < num_permutations; i++)
    {
        results[i + 1] = 0;
    }

    double best_res = 0;
    generate_permutations(tasks_arr, 0, n, tasks, results, &current, best, &best_res);

    /*
    printf("Best result: %f\n", best_res);
    for (int i = 0; i < n; i++)
    {
        Task *task = tasks[best[i]];
        int slack = task->deadline - task->duration;
        double utilization = task->utilization;
        printf("%d: Task %d\tT=%d\tC=%d\tS=%d\tU=%.3f\n", i + 1, task->id, task->period, task->duration, slack, utilization);
    }
    */

    free_tasks(tasks, n);

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
    printf("OPARSM=%.3f,", rm[0] / max);
    printf("Med=%.3f,", median / max);
    printf("Min=%.3f,", min / max);
    printf("Avg=%.3f", avg / max);
    printf("\n");
    
    return 0;
}
