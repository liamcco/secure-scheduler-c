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
void sim(int n, int m, Task **tasks, int *allocation, double *result)
{
    int hyper_period = 3000;

    Processor *processor = init_processor_custom(m, &init_scheduler_ts); // Choose schedudler here

    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;
    processor->horizontal = 0;
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
void generate_allocations(int n, int m, int task, int allocation[], int max_bin, Task **tasks, double results[], int *current)
{
    if (task == n)
    { // Base case: all tasks assigned
        /*
        printf("\n[ ");
        for (int i = 0; i < n; i++)
        {
            printf("%d ", allocation[i]);
        }
        printf("]\n");
        */
        double result[3];
        for (int i = 0; i < 3; i++)
        {
            result[i] = 0;
        }
        sim(n, m, tasks, allocation, result);
        if (result[0] == 0)
        {
            return;
        }
        results[*current] = result[0];
        *current = *current + 1;
        //printf("Result: %.3f\n", result[0]);
        return;
    }

    // Assign task to an existing bin or the next available bin
    for (int bin = 1; bin <= max_bin + 1 && bin <= m; bin++)
    {
        allocation[task] = bin;
        generate_allocations(n, m, task + 1, allocation, (bin > max_bin) ? max_bin + 1 : max_bin, tasks, results, current);
    }
}

// Wrapper function to start recursion
void generate_unique_allocations(int n, int m, Task **tasks, double results[], int *current)
{
    int allocation[n]; // Array to store current allocation
    generate_allocations(n, m, 0, allocation, 0, tasks, results, current);
}

// Function to compute factorial of a number
long long factorial(int x)
{
    long long result = 1;
    for (int i = 2; i <= x; i++)
    {
        result *= i;
    }
    return result;
}

// Function to compute T(n, m) = (m^n) / m!
long long count_unique_allocations(int n, int m)
{
    if (m > n)
        m = n; // More bins than tasks is redundant
    long long total_assignments = 1;

    // Compute m^n (total assignments without considering bin symmetry)
    for (int i = 0; i < n; i++)
    {
        total_assignments *= m;
    }

    // Divide by m! to account for interchangeable bins
    return total_assignments / factorial(m);
}

// Comparison function for qsort
int compare(const void *a, const void *b)
{
    double diff = *(double *)a - *(double *)b;
    return (diff > 0) - (diff < 0); // Returns -1, 0, or 1
}

// Example usage
int main(void)
{
    // printf("Starting...\n");
    srand(time(NULL) ^ clock());
    int n = 5; // Number of tasks
    int m = 3; // Number of bins
    long long total_assignments = count_unique_allocations(n, m);
    // printf("Total unique assignments: %lld\n", total_assignments);
    double U = 0.5;

    int hyper_period = 3000;

    Task **tasks = generate_task_set(n, U * m, hyper_period, 1, 50);
    prioritize(tasks, n, &IU); // <- choose order in which to assign tasks (can also happen in your partitioning algorithm)

    double actual_U = 0;
    for (int i = 0; i < n; i++)
    {
        actual_U += tasks[i]->utilization;
    }

    // compare with WF-DU
    Processor *processor = init_processor_custom(m, &init_scheduler_ts);

    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;
    processor->horizontal = 0;
    processor->analyze = &analyze_simulation;

    // int load_was_successful = load_tasks_with_algorithm_argument(processor, tasks, n, &ff_50percent_custom, 0.50); 
    int load_was_successful = load_tasks(processor, tasks, n, &wf); // <- Load tasks with any partitioning algorithm

    if (!load_was_successful)
    {
        free_processor(processor);
        free_tasks(tasks, n);
        return 0;
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
    }*/

    double result[3];
    for (int i = 0; i < 3; i++)
    {
        result[i] = 0;
    }

    double results[total_assignments + 1];
    int current = 0;

    run(processor, hyper_period, 1000, result);
    results[current] = result[0];
    current++;

    free_processor(processor);

    double ff = results[0];
    printf("CP1=%.3f\n", ff);

    // Do everyting
    generate_unique_allocations(n, m, tasks, results, &current);

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
    printf("CP1=%.3f,", ff / max);
    printf("Med=%.3f,", median / max);
    printf("Min=%.3f,", min / max);
    printf("Avg=%.3f", avg / max);
    printf("\n\n");

    return 0;
}
