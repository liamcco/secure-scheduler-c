#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "task.h"
#include "processor.h"
#include "partition_algorithms.h"
#include "taskset.h"

// Example usage
int main(void)
{
    srand(time(NULL) ^ clock());

    int hyper_period = 3000;

    Task **tasks = generate_task_set(5, 0.5, hyper_period, 1, 50);

    Processor *processor = init_processor_custom(1, &init_scheduler_ts);

    load_tasks(processor, tasks, 5, &ff);

    run(processor, hyper_period, 1000, NULL);

    free_tasks(tasks, 5);

    return 0;
}
