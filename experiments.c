#include <stdio.h>
#include <math.h>

#include "experiments.h"

void analyze_simulation(Processor *processor, double *result)
{
    int horizontal = 1;
    int log_attack_data = processor->log_attack_data;
    int log_timeslot_data = processor->log_timeslot_data;

    SimulationData *sim_data = processor->simulation;

    AttackData *attack_data = sim_data->attack_data;
    int num_tasks = processor->all_tasks->num_tasks;

    Task **tasks = processor->all_tasks->tasks;

    if (log_attack_data)
    {
        double current_max_p_a = 0;
        double task_max_p_a[num_tasks];

        double current_max_p_p = 0;
        double task_max_p_p[num_tasks];

        double current_max_p_pinch = 0;
        double task_max_p_pinch[num_tasks];

        for (int i = 0; i < num_tasks; i++)
        {
            AttackData *data = &attack_data[i];
            int num_instances = data->num_instances;
            double p_anterior_max = 0;
            double p_posterior_max = 0;
            double p_pincher_max = 0;

            for (int j = 0; j < num_tasks; j++)
            {
                // if (i == j)
                //     continue;

                if (tasks[j]->trusted)
                {
                    continue;
                }

                if (horizontal && tasks[j]->c_id != tasks[i]->c_id)
                {
                    continue;
                }

                if (data->anterior[j])
                {
                    double p_anterior = (double)data->anterior[j] / num_instances;
                    if (p_anterior > p_anterior_max)
                        p_anterior_max = p_anterior;
                    // printf("P(%d ant by %d) %f\n", tasks[i]->id, tasks[j]->id, p_anterior);
                }
                if (data->posterior[j])
                {
                    double p_posterior = (double)data->posterior[j] / num_instances;
                    if (p_posterior > p_posterior_max)
                        p_posterior_max = p_posterior;
                    // printf("P(%d post by %d): %f\n", tasks[i]->id, tasks[j]->id, p_posterior);
                }
                if (data->pincher[j])
                {
                    double p_pincher = (double)data->pincher[j] / num_instances;
                    if (p_pincher > p_pincher_max)
                        p_pincher_max = p_pincher;
                    // printf("P(%d pinch by %d): %f\n", tasks[i]->id, tasks[j]->id, p_pincher);
                }
            }

            task_max_p_a[i] = p_anterior_max;
            task_max_p_p[i] = p_posterior_max;
            task_max_p_pinch[i] = p_pincher_max;

            //  printf("P(%d ant): %f\n", i, p_anterior_max);
            //  printf("P(%d post): %f\n", i, p_posterior);
            //  printf("P(%d pinch): %f\n", i, p_pincher);
            if (p_anterior_max > current_max_p_a)
                current_max_p_a = p_anterior_max;
            if (p_posterior_max > current_max_p_p)
                current_max_p_p = p_posterior_max;
            if (p_pincher_max > current_max_p_pinch)
                current_max_p_pinch = p_pincher_max;
        }

        double p_a_security = 1;
        double p_p_security = 1;
        double p_pinch_security = 1;

        for (int i = 0; i < num_tasks; i++)
        {
            if (1 - task_max_p_a[i] < p_a_security)
                p_a_security = 1 - task_max_p_a[i];

            if (1 - task_max_p_p[i] < p_p_security)
                p_p_security = 1 - task_max_p_p[i];

            if (1 - task_max_p_pinch[i] < p_pinch_security)
                p_pinch_security = 1 - task_max_p_pinch[i];
        }

        *result = p_a_security;
        // printf("Max Pan: %f\n", current_max_p_a);
        // printf("Max Ppo: %f\n", current_max_p_p);
        // printf("Max Ppi: %f\n", current_max_p_pinch);
        // printf("\n");
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
            for (int j = 1; j < num_tasks + 1; j++) // Do not count idle
            {
                double p = (double)t_data[i * num_tasks + j] / (num_periods * sim_data->num_cores);
                if (p > 0)
                    entropy -= p * log2(p);
            }
            total_entropy += entropy;
        }

        // printf("%.2f\n", total_entropy);
        *result = total_entropy;
    }

    return;
}
