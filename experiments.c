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
            int num_instances = attack_data[i].num_instances;
            double p_anterior = 0;
            double p_posterior = 0;
            double p_pincher = 0;

            for (int j = 0; j < num_tasks; j++)
            {
                // if (i == j)
                //     continue;

                if (attack_data[i].anterior[j])
                {
                    p_anterior += (double)attack_data[i].anterior[j] / num_instances;
                    // printf("P(%d ant by %d) %f\n", i, j, (double)attack_data[i].anterior[j] / num_instances);
                }
                if (attack_data[i].posterior[j])
                {
                    p_posterior += (double)attack_data[i].posterior[j] / num_instances;
                    // printf("P(%d post by %d): %f\n", i, j, (double)attack_data[i].posterior[j] / num_instances);
                }
                if (attack_data[i].pincher[j])
                {
                    p_pincher += (double)attack_data[i].pincher[j] / num_instances;
                    // printf("P(%d pinch by %d): %f\n", i, j, (double)attack_data[i].pincher[j] / num_instances);
                }
            }

            printf("P(%d ant): %f\n", i, p_anterior);
            printf("P(%d post): %f\n", i, p_posterior);
            printf("P(%d pinch): %f\n", i, p_pincher);
        }
    }

    if (log_timeslot_data)
    {
        double total_entropy = 0;
        int *t_data = sim_data->timeslots;
        int hyperperiod = sim_data->hyperperiod;
        int num_periods = sim_data->time / hyperperiod;
        int num_tasks = sim_data->num_tasks;

        for (int i = 0; i < hyperperiod; i++)
        {
            double entropy = 0;
            for (int j = 0; j < num_tasks + 1; j++)
            {
                double p = (double)t_data[i * num_tasks + j] / (num_periods * sim_data->num_cores);
                if (p > 0)
                    entropy -= p * log2(p);
            }
            total_entropy += entropy;
        }

        double U = 0;
        for (int i = 0; i < num_tasks; i++)
        {
            Task *task = processor->all_tasks->tasks[i];
            U += task->utilization;
        }

        printf("%.2f,%.2f,%d\n", U, total_entropy, num_tasks);
    }

    return;
}