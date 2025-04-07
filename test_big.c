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
void sim_allocation(int n, int m, Task **tasks, int *allocation, double *result)
{
    int hyper_period = 3000;

    Processor *processor = init_processor_custom(m, &init_scheduler_ts); // Choose schedudler here

    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;
    processor->migration = 0;
    processor->analyze = &analyze_simulation;

    int load_was_successful = load_tasks_from_allocation(processor, tasks, n, allocation); // Attempts to load task according to given allocation

    if (!load_was_successful)
    {
        free_processor(processor);
        return;
    }

    // print task partition
    /*for (int i = 0; i < m; i++)
    {
        TaskGroup *group = processor->tasks->task_groups[i];
        printf("Core %d: ", i);
        printf("U=%.2f - ", group->utilization);
        for (int j = 0; j < group->num_tasks; j++)
        {
            printf("%d\t", group->tasks[j]->id);
        }

        printf("\n");
    }*/

    // Reset tasks
    for (int i = 0; i < n; i++)
    {
        reset(tasks[i]);
    }
    
    run(processor, hyper_period, 1000, result);

    free_processor(processor);
    return;
}

// Recursive function to generate unique task assignments
void generate_allocations(int n, int m, int task, int allocation[], int max_bin, Task **tasks, double results_h[], double results_v[], int *current)
{
    if (task == n)
    { // Base case: all tasks assigned
        
        /*printf("\n[ ");
        for (int i = 0; i < n; i++)
        {
            printf("%d ", allocation[i]);
        }
        printf("]\n"); */
        
        
        double result[8];
        for (int i = 0; i < 8; i++)
        {
            result[i] = -1;
        }

        sim_allocation(n, m, tasks, allocation, result);

        if (result[6] == -1)
        {
            return;
        }

        results_h[*current] = result[6];
        results_v[*current] = result[7];
        *current = *current + 1;
        //printf("Result: %.3f\n", result[6]);
        return;
    }

    // Assign task to an existing bin or the next available bin
    for (int bin = 1; bin <= max_bin + 1 && bin <= m; bin++)
    {
        allocation[task] = bin;
        generate_allocations(n, m, task + 1, allocation, (bin > max_bin) ? max_bin + 1 : max_bin, tasks, results_h, results_v, current);
    }
}

// Wrapper function to start recursion
void generate_unique_allocations(int n, int m, Task **tasks, double results_h[], double results_v[], int *current)
{
    int allocation[n]; // Array to store current allocation
    generate_allocations(n, m, 0, allocation, 0, tasks, results_h, results_v, current);
}

// Stirling numbers of the second kind using dynamic programming
long long stirling(int n, int k)
{
    if (n == k || k == 1)
        return 1;
    if (k > n)
        return 0;

    return k * stirling(n - 1, k) + stirling(n - 1, k - 1);
}

// Total number of unique assignments
long long count_unique_allocations(int n, int m)
{
    long long total = 0;
    for (int k = 1; k <= m; k++)
    {
        total += stirling(n, k);
    }
    return total;
}

// Comparison function for qsort
int compare(const void *a, const void *b)
{
    double diff = *(double *)a - *(double *)b;
    return (diff > 0) - (diff < 0); // Returns -1, 0, or 1
}

int sim_partition(Task **tasks, int n, int m, double *result, Partition *(*partition_algorithm)(Task **, int, int), int (*compare)(const void *, const void *))
{
    // Initialize processor and load tasks
    Processor *processor = init_processor_custom(m, &init_scheduler_ts);
    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;
    processor->migration = 0;
    processor->analyze = &analyze_simulation;

    // Load tasks according to the given allocation
    prioritize(tasks, n, compare);
    int load_was_successful = load_tasks(processor, tasks, n, partition_algorithm);
    if (!load_was_successful)
    {
        free_processor(processor);
        return 0;
    }

    for (int i = 0; i < n; i++)
    {
        reset(tasks[i]);
    }
    // Run simulation
    run(processor, 3000, 1000, result);
    //printf("Result: %.3f\n", result[6]);
    // Reset tasks

    free_processor(processor);
    return 1;
}

int sim_partition_ff50(Task **tasks, int n, int m, double *result)
{
    // Initialize processor and load tasks
    Processor *processor = init_processor_custom(m, &init_scheduler_ts);
    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;
    processor->migration = 0;
    processor->analyze = &analyze_simulation;

    // Load tasks according to the given allocation
    prioritize(tasks, n, compare);
    int load_was_successful = load_tasks_with_algorithm_argument(processor, tasks, n, &ff_50percent_custom, 0.5);
    if (!load_was_successful)
    {
        free_processor(processor);
        return 0;
    }

    for (int i = 0; i < n; i++)
    {
        reset(tasks[i]);
    }
    // Run simulation
    run(processor, 3000, 1000, result);
    //printf("Result: %.3f\n", result[6]);
    // Reset tasks

    free_processor(processor);
    return 1;
}

// Example usage
int main(void)
{
    // printf("Starting...\n");
    srand(time(NULL) ^ clock());
    int n = 25; // Number of tasks
    int m = 4; // Number of bins
    long long total_assignments = count_unique_allocations(n, m);
    //printf("Total unique assignments: %lld\n", total_assignments);
    //printf("n=%d,m=%d,Total unique assignments: %lld\n", n, m, total_assignments);
    for (int u = 2; u < 81; u++)
    {
    double U = (double)u / 100.0;
    int hyper_period = 3000;

    Task **tasks = generate_task_set(n, U * m, hyper_period, 1, 50);
    double actual_U = 0;
    for (int i = 0; i < n; i++)
    {
        actual_U += tasks[i]->utilization;
    }

    // print task partition
    /*for (int i = 0; i < m; i++)
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

    printf("U=%.2f,", actual_U);
    double result[8];
    for (int i = 0; i < 8; i++)
    {
        result[i] = 0;
    }

    int success = sim_partition(tasks, n, m, result, &wf, &RM);
    if (success) {
        printf("WF-RM_h=%.3f,", result[6]);
        printf("WF-RM_v=%.3f,", result[7]);
    }
    success = sim_partition(tasks, n, m, result, &bf, &RM);
    if (success) {
        printf("BF-RM_h=%.3f,", result[6]);
        printf("BF-RM_v=%.3f,", result[7]);
    }
    success = sim_partition(tasks, n, m, result, &wf, &IU);
    if (success) {
        printf("WF-IU_h=%.3f,", result[6]);
        printf("WF-IU_v=%.3f,", result[7]);
    }
    success = sim_partition(tasks, n, m, result, &bf, &IU);
    if (success) {
        printf("BF-IU_h=%.3f,", result[6]);
        printf("BF-IU_v=%.3f,", result[7]);
    }
    success = sim_partition(tasks, n, m, result, &wf, &DU);
    if (success) {
        printf("WF-DU_h=%.3f,", result[6]);
        printf("WF-DU_v=%.3f,", result[7]);
    }
    success = sim_partition(tasks, n, m, result, &bf, &DU);
    if (success) {
        printf("BF-DU_h=%.3f,", result[6]);
        printf("BF-DU_v=%.3f,", result[7]);
    }
    success = sim_partition(tasks, n, m, result, &ff, &RM);
    if (success) {
        printf("FF-RM_h=%.3f,", result[6]);
        printf("FF-RM_v=%.3f,", result[7]);
    }
    success = sim_partition(tasks, n, m, result, &ff, &IU);
    if (success) {
        printf("FF-IU_h=%.3f,", result[6]);
        printf("FF-IU_v=%.3f,", result[7]);
    }
    success = sim_partition(tasks, n, m, result, &ff, &DU);
    if (success) {
        printf("FF-DU_h=%.3f,", result[6]);
        printf("FF-DU_v=%.3f,", result[7]);
    }
    success = sim_partition(tasks, n, m, result, &wfminm, &DU);
    if (success) {
        printf("WF-minm_h=%.3f,", result[6]);
        printf("WF-minm_v=%.3f,", result[7]);
    }
    success = sim_partition_ff50(tasks, n, m, result);
    if (success) {
        printf("FF-50_h=%.3f,", result[6]);
        printf("FF-50_v=%.3f,", result[7]);
    }
    

    free_tasks(tasks, n);
    printf("\n");

    }

    return 0;
}
