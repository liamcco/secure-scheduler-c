#ifndef SIMULATION_H
#define SIMULATION_H

#include "processor.h"

// Forward declare Processor to avoid compilation errors
struct Processor;
typedef struct Processor Processor;

typedef struct AttackData
{
    int *anterior;
    int *posterior;
    int *pincher;
    int *current_anteriors;
    int *current_posteriors;
} AttackData;

typedef struct TimeslotData
{
    int *timeslots;
    int num_tasks;
    int hyper_period;
} TimeslotData;

typedef struct SimulationData
{
    int *schedule;
    AttackData *attack_data; // Single array instead of double pointer
    TimeslotData *timeslot_data;
    int hyperperiod;
    int num_cores;
    int time;
} SimulationData;

void setup_simulation(Processor *processor, int time);
void log_execution(Processor *processor, int time);
void check_new_tasks_for_attacks(Processor *processor);
int calculate_hyperperiod(struct Task **tasks, int num_tasks);
void teardown_simulation(Processor *processor);

#endif // SIMULATION_H