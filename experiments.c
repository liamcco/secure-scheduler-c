#include <stdio.h>
#include <math.h>

#include "experiments.h"

void analyze_simulation(Processor *processor)
{
    int log_attack_data = processor->log_attack_data;
    int log_timeslot_data = processor->log_timeslot_data;

    SimulationData *sim_data = processor->simulation;

    AttackData *attack_data = sim_data->attack_data;
    int num_tasks = processor->all_tasks->num_tasks;

    if (log_attack_data)
    {
        for (int i = 0; i < num_tasks; i++)
        {
            for (int j = 0; j < num_tasks; j++)
            {
                if (i == j)
                    continue;

                if (attack_data[i].anterior[j])
                    printf("Task %d has %d anterior attacks from task %d\n", i, attack_data[i].anterior[j], j);
                if (attack_data[i].posterior[j])
                    printf("Task %d has %d posterior attacks from task %d\n", i, attack_data[i].posterior[j], j);
                if (attack_data[i].pincher[j])
                    printf("Task %d has %d pincher attacks from task %d\n", i, attack_data[i].pincher[j], j);
            }
        }
    }

    if (log_timeslot_data)
    {
        double total_entropy = 0;
        for (int i = 0; i < processor->m; i++)
        {
            TimeslotData *t_data = &sim_data->timeslot_data[i];
            int num_periods = sim_data->time / t_data->hyper_period;

            for (int j = 0; j < t_data->hyper_period; j++)
            {
                double entropy = 0;
                for (int k = 0; k < t_data->num_tasks + 1; k++)
                {
                    double p = (double)t_data->timeslots[j * t_data->num_tasks + k] / num_periods;
                    if (p > 0)
                        entropy -= p * log2(p);
                }
                total_entropy += entropy;
            }
        }

        double U = 0;
        for (int i = 0; i < num_tasks; i++)
        {
            Task *task = processor->all_tasks->tasks[i];
            U += task->utilization;
        }

        printf("U: %.2f, total_entropy: %.2f, n: %d\n", U, total_entropy, num_tasks);
    }

    return;
}