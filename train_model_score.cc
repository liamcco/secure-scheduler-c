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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CORES 2
#define MAX_TASKS 5



// Example usage
int main(int argc, char **argv)
{
    srand(time(NULL) ^ clock());
    if (argc != 2)
    {
        printf("Usage: %s <task_set>\n", argv[0]);
        return 1;
    }


    const char *ptr = argv[1];
    int total_num_tasks = 0;

    while (*ptr) {
        if (*ptr == '(') {  // Task detected
            total_num_tasks++;
        }
        ptr++;
    }

    int allocation[total_num_tasks];
    Task **task_set = malloc(total_num_tasks * sizeof(Task *));

    ptr = argv[1];
    int tasks_added = 0;
    int current_partition = -1; // Start before first partition
    
    while (*ptr) {
        if (*ptr == '[' && *(ptr + 1) == '[') {  
            ptr++;  // Skip the first '['
            continue;
        } else if (*ptr == '[' && *(ptr + 1) == '(') {  
            current_partition++;  // Enter new partition
        } else if (*ptr == '[' && *(ptr + 1) == ']') {  
            current_partition++;  // Empty partition detected
        } else if (*ptr == '(') {  
            int T, C;
            if (tasks_added < total_num_tasks && sscanf(ptr, "(%d,%d)", &T, &C) == 2) {
                task_set[tasks_added] = init_task(T, C);
                allocation[tasks_added] = current_partition + 1;
                tasks_added++;
            }
        }
        ptr++;
    }
    
    int m = current_partition + 1; // Number of bins
    int hyper_period = 300;

    Processor *processor = init_processor_custom(m, &init_scheduler_ts);

    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;
    processor->analyze = &analyze_simulation;

    int load_was_successful = load_tasks_from_allocation(processor, task_set, total_num_tasks, allocation);

    if (!load_was_successful)
    {
        free_processor(processor);
        free_tasks(task_set, total_num_tasks);
        printf("0\n");
        return 0;
    }

    double result[8];

    run(processor, hyper_period, 1000, result);

    free_processor(processor);

    double model_score = result[6];

    processor = init_processor_custom(m, &init_scheduler_ts);

    processor->log_attack_data = 0;
    processor->log_timeslot_data = 1;
    processor->analyze = &analyze_simulation;

    load_was_successful = load_tasks_with_algorithm_argument(processor, task_set, total_num_tasks, &ff_50percent_custom, 0.50);
    if (!load_was_successful)
    {
        
        printf("2\n");
        return 0;
    }

    run(processor, hyper_period, 1000, result);

    free_processor(processor);
    free_tasks(task_set, total_num_tasks);

    double baseline = result[6];

    double score = model_score / baseline;

    printf("%.2f\n", score);

    return 0;
}
