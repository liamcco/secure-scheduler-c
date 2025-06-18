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

int try_simulation(Task **tasks, int n, double* result)
{
    Processor *processor = init_processor_custom(1, &init_scheduler_nosort); // Unicore

    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;
    processor->analyze = &analyze_simulation; // <- This is the function that calculates the result (experiments.c)

    int load_successful = load_tasks(processor, tasks, n, &ff_nosort); // <- ff_nosort will fail if the priority assignment is not feasible
                                                                       // (ff assumes RM)
    if (!load_successful)
    {
        free_processor(processor);
        return 0;
    }

    // Reset tasks if task set is to be reused
    for (int i = 0; i < n; i++)
    {
        reset(tasks[i]);
    }

    run(processor, 3000, 1000, result);

    free_processor(processor);
    return 1;
}

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Recursive function to generate permutations
void generate_permutations(int arr[], int start, int n, Task **tasks, double results_h[], double results_v[], int *current)
{
    if (start == n)
    {
        // Try running simulation with this allocation
        Task **tasks_copy = (Task **)malloc(sizeof(Task *) * n);
        for (int i = 0; i < n; i++)
        {
            tasks_copy[i] = tasks[arr[i]];
        }
        double result[8];
        for (int i = 0; i < 8; i++)
        {
            result[i] = -1;
        }
        int success = try_simulation(tasks_copy, n, result);
        if (!success)
        {
            free(tasks_copy);
            return;
        }

        results_h[*current] = result[6];
        results_v[*current] = result[7];
        *current = *current + 1;

        free(tasks_copy);
        // printf("Results: %.2f, %.2f\n", result[6], result[7]);

        return;
    }

    for (int i = start; i < n; i++)
    {
        // Swap to fix one element at the current position
        swap(&arr[start], &arr[i]);

        // Recur for the next position
        generate_permutations(arr, start + 1, n, tasks, results_h, results_v, current);

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

    for (int i = 0; i < 8; i++) {
    for (int u = 2; u < 85; u++) {

    double U = (double)u / 100.0;
    double actual_U;

    Task **tasks = generate_task_set(n, U, hyper_period, 1, 50);

    actual_U = 0;
    for (int j = 0; j < n; j++)
    {
        actual_U += tasks[j]->utilization; // Calculate actual utilization
    }

    int feasible = OPA(tasks, n, &RM); //
    if (!feasible)
    {
        free_tasks(tasks, n);
        continue;
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

    // Iterate through all possible task priority assignments

    int tasks_arr[n];
    for (int i = 0; i < n; i++)
    {
        tasks_arr[i] = i;
    }

    int num_permutations; // = factorial(n);
    num_permutations = 1;
    for (int i = 1; i <= n; i++)
    {
        num_permutations *= i;
    }

    int current = 0;
    double results_h[num_permutations];
    double results_v[num_permutations];

    generate_permutations(tasks_arr, 0, n, tasks, results_h, results_v, &current);

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

    // sort results
    qsort(results_h, current, sizeof(double), &compare);
    qsort(results_v, current, sizeof(double), &compare);

    double min_h = results_h[0];
    double min_v = results_v[0];
    double max_h = results_h[current - 1];
    double max_v = results_v[current - 1];
    double avg_h = 0;
    double avg_v = 0;
    for (int i = 0; i < current; i++)
    {
        avg_h += results_h[i];
        avg_v += results_v[i];
    }
    avg_h /= current;
    avg_v /= current;

    printf("U=%.2f,", actual_U);
    printf("Min_h=%.3f,", min_h / max_h);
    printf("Min_v=%.3f,", min_v / max_v);
    printf("Avg_h=%.3f,", avg_h / max_h);
    printf("Avg_v=%.3f,", avg_v / max_v);


    double result[8];
    for (int i = 0; i < 8; i++)
    {
        result[i] = 0;
    }

    // order tasks
    OPA(tasks, n, &RM);
    // run simulation with RM
    int success = try_simulation(tasks, n, result); // Run simulation with RM
    if (success)
    {
        printf("RM_h=%.3f,", result[6] / max_h);
        printf("RM_v=%.3f,", result[7] / max_v);
    }

    OPA(tasks, n, &DU);
    success = try_simulation(tasks, n, result); // Run simulation with DU
    if (success)
    {
        printf("DU_h=%.3f,", result[6] / max_h);
        printf("DU_v=%.3f,", result[7] / max_v);
    }

    OPA(tasks, n, &IU);
    success = try_simulation(tasks, n, result); // Run simulation with IU
    if (success)
    {
        printf("IU_h=%.3f,", result[6] / max_h);
        printf("IU_v=%.3f,", result[7] / max_v);
    }

    OPA(tasks, n, &RSM);
    success = try_simulation(tasks, n, result); // Run simulation with RSM
    if (success)
    {
        printf("RSM_h=%.3f,", result[6] / max_h);
        printf("RSM_v=%.3f,", result[7] / max_v);
    }

    OPA(tasks, n, &SM);
    success = try_simulation(tasks, n, result); // Run simulation with SM
    if (success)
    {
        printf("SM_h=%.3f,", result[6] / max_h);
        printf("SM_v=%.3f,", result[7] / max_v);
    }

    printf("\n");
    free_tasks(tasks, n);

    }
    }
    
    return 0;
}
