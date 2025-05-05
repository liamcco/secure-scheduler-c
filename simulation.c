#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "simulation.h"

void setup_simulation(Processor *processor, int hyperperiod, int num_hyperperiods)
{

    SimulationData *data = malloc(sizeof(SimulationData));

    int time = hyperperiod * num_hyperperiods;

    data->schedule = calloc(processor->m * time, sizeof(int *));
    data->num_cores = processor->m;
    data->time = time;
    data->hyperperiod = hyperperiod;
    data->num_tasks = processor->all_tasks->num_tasks;

    int num_tasks = data->num_tasks;

    // Allocate attack data as a contiguous block
    data->attack_data = malloc(num_tasks * sizeof(AttackData));
    for (int i = 0; i < num_tasks; i++)
    {
        AttackData *attack_data = &data->attack_data[i];
        attack_data->anterior = calloc(num_tasks, sizeof(int));
        attack_data->posterior = calloc(num_tasks, sizeof(int));
        attack_data->pincher = calloc(num_tasks, sizeof(int));
        attack_data->current_anteriors = calloc(num_tasks, sizeof(int));
        attack_data->current_posteriors = calloc(num_tasks, sizeof(int));
        attack_data->num_instances = 0;
    }

    // Allocate attack data as a contiguous block
    data->attack_data_h = malloc(num_tasks * sizeof(AttackData));
    for (int i = 0; i < num_tasks; i++)
    {
        AttackData *attack_data = &data->attack_data_h[i];
        attack_data->anterior = calloc(num_tasks, sizeof(int));
        attack_data->posterior = calloc(num_tasks, sizeof(int));
        attack_data->pincher = calloc(num_tasks, sizeof(int));
        attack_data->current_anteriors = calloc(num_tasks, sizeof(int));
        attack_data->current_posteriors = calloc(num_tasks, sizeof(int));
        attack_data->num_instances = 0;
    }

    data->timeslots = malloc(processor->m * sizeof(int *));
    // Allocate timeslot data
    for (int i = 0; i < processor->m; i++)
    {
        data->timeslots[i] = calloc(data->hyperperiod * (num_tasks + processor->m), sizeof(int));
    }

    processor->simulation = data;
}

void teardown_simulation(Processor *processor)
{
    SimulationData *data = processor->simulation;
    int num_tasks = processor->all_tasks->num_tasks;

    for (int i = 0; i < num_tasks; i++)
    {
        AttackData *attack_data = &data->attack_data[i];
        free(attack_data->anterior);
        free(attack_data->posterior);
        free(attack_data->pincher);
        free(attack_data->current_anteriors);
        free(attack_data->current_posteriors);
    }
    free(data->attack_data);

    for (int i = 0; i < num_tasks; i++)
    {
        AttackData *attack_data = &data->attack_data_h[i];
        free(attack_data->anterior);
        free(attack_data->posterior);
        free(attack_data->pincher);
        free(attack_data->current_anteriors);
        free(attack_data->current_posteriors);
    }
    free(data->attack_data_h);

    for (int i = 0; i < processor->m; i++)
    {
        free(data->timeslots[i]);
    }
    free(data->timeslots);

    free(data->schedule);
    free(data);
}

void check_new_tasks_for_attacks(TaskGroup *all_tasks, AttackData* attack_data)
{
    int num_tasks = all_tasks->num_tasks;
    Task **tasks = all_tasks->tasks;

    // Enter Attack Data
    for (int i = 0; i < num_tasks; i++)
    {
        if (is_new(tasks[i]))
        {
            AttackData *data = &attack_data[i];
            data->num_instances++;

            // Summarize current anteriors
            for (int j = 0; j < num_tasks; j++)
            {
                int anterior = data->current_anteriors[j];
                int posterior = data->current_posteriors[j];

                data->anterior[j] += anterior;
                data->posterior[j] += posterior;
                data->pincher[j] += anterior && posterior;
            }

            // Reset current anteriors
            // Reset current anteriors and posteriors efficiently
            memset(data->current_anteriors, 0, num_tasks * sizeof(int));
            memset(data->current_posteriors, 0, num_tasks * sizeof(int));
        }
    }
}

void log_execution(Processor *processor, int time)
{
    int log_schedule = processor->log_schedule;
    int log_attack_data = processor->log_attack_data;
    int log_timeslot_data = processor->log_timeslot_data;

    SimulationData *sim_data = processor->simulation;

    // Enter Attack Data

    if (log_attack_data) {
        check_new_tasks_for_attacks(processor->all_tasks, sim_data->attack_data);
        check_new_tasks_for_attacks(processor->all_tasks, sim_data->attack_data_h);
    }
        
    AttackData *attack_data = sim_data->attack_data;
    AttackData *attack_data_h = sim_data->attack_data_h;
    Task **tasks = processor->all_tasks->tasks;
    int num_tasks = processor->all_tasks->num_tasks;

    // printf("\n%d:\t", time);

    // Enter Schedule
    for (int i = 0; i < processor->m; i++)
    {
        // Schedule
        Task *task = processor->ready_tasks[i];
        // printf("%d\t", task->id);

        if (log_schedule)
            sim_data->schedule[time + i] = task->id;

        // Timeslot data
        if (log_timeslot_data)
        {
            int idx = task->id + processor->m - 1;
            sim_data->timeslots[i][(time % sim_data->hyperperiod) * (sim_data->num_tasks + processor->m)+ idx]++;
        }

        // Attack Data
        if (log_attack_data)
        {
            int executed_idx = processor->ready_tasks[i]->idx;
            if (executed_idx == -1)
                continue;

            for (int j = 0; j < num_tasks; j++)
            {
                int will_execute = 0;
                for (int k = 0; k < processor->m; k++)
                {
                    if (processor->ready_tasks[k]->idx == j)
                    {
                        will_execute = 1;
                        break;
                    }
                }
                if (will_execute)
                    continue;

                int same_core = tasks[j]->c_idx == task->c_idx;
                // Only considers attacks on same core...
                if (is_fresh(tasks[j]) && same_core) {
                    attack_data[j].current_anteriors[executed_idx] = 1;
                    if (same_core)
                        attack_data_h[j].current_anteriors[executed_idx] = 1;
                }
                if (is_complete(tasks[j]) && same_core) {
                    attack_data[j].current_posteriors[executed_idx] = 1;
                    if (same_core)
                        attack_data_h[j].current_posteriors[executed_idx] = 1;
                }
            }
        }
    }
}

// Function to calculate the hyperperiod of a set of tasks
int calculate_hyperperiod(struct Task **tasks, int num_tasks)
{
    int hyperPeriod = 1;
    for (int i = 0; i < num_tasks; i++)
    {
        hyperPeriod *= tasks[i]->period / gcd(tasks[i]->period, hyperPeriod);
    }
    return hyperPeriod;
}
