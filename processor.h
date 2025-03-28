#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "task.h"
#include "core.h"
#include "partition_algorithms.h"
#include "simulation.h"

// Forward declare Processor to avoid compilation errors
struct Simiulationdata;
typedef struct SimulationData SimulationData;

typedef struct Processor
{
    Partition *tasks;

    TaskGroup *all_tasks;

    Core **cores;
    int m;

    Task **ready_tasks;

    int log_schedule;
    int log_attack_data;
    int log_timeslot_data;
    int horizontal;
    void (*analyze)(struct Processor *processor, double *result);

    SimulationData *simulation;

} Processor;

Processor *init_processor(int m);
Processor *init_processor_custom(int m, Scheduler *(*init_scheduler)(void));
void free_processor(Processor *processor);
Task **schedule_tasks(Processor *processor);
void time_step_processor(Processor *processor);
int load_tasks(Processor *processor, Task **tasks, int num_tasks, Partition *(partition_algorithm)(Task **, int, int));
int load_tasks_with_algorithm_argument(Processor *processor, Task **tasks, int num_tasks, Partition *(partition_algorithm)(Task **, int, int, double), double);
int load_tasks_from_allocation(Processor *processor, Task **tasks, int num_tasks, int *allocation);
int run(Processor *processor, int hyperperiod, int num_hyperperiods, double *result);

#endif // PROCESSOR_H
