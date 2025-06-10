#include <stdio.h>
#include <math.h>

#include "experiments.h"

void calculate_schedule_entropy_horizontal(Processor *processor, double *result)
{
    SimulationData *sim_data = processor->simulation;
    double entropies[processor->m];
    int hyperperiod = sim_data->hyperperiod;
    int num_periods = sim_data->time / hyperperiod;
    int num_all_tasks = sim_data->num_tasks + processor->m;
        
    for (int i = 0; i < processor->m; i++)
    {
        int *t_data = sim_data->timeslots[i];
        double total_entropy = 0;
        entropies[i] = 0;

        TaskGroup* tasks = processor->tasks->task_groups[i];
        int num_tasks = tasks->num_tasks;

        if (num_tasks == 0)
            continue;

        for (int k = 0; k < hyperperiod; k++)   // for every timeslot
        {
            double entropy = 0;

            for (int j = 0; j < num_all_tasks; j++)
            {
                double p = (double)t_data[k * num_all_tasks + j] / (num_periods);
                if (p > 0)
                    entropy -= p * log2(p);
            }
            total_entropy += entropy;
                
        }
        entropies[i] = total_entropy;
    }

    double total_combned_entropy = 1;
    int m_actual = processor->m;
    for (int i = 0; i < processor->m; i++)
    {
        if (entropies[i] == 0) {
            m_actual--;
            continue;
        }
        total_combned_entropy *= entropies[i];
    }
    //Take the mth root of the product of the entropies
    if (m_actual == 0) {
        printf("Error: no entropy\n");
        *result = 0;
        return;
    }

    total_combned_entropy = pow(total_combned_entropy, 1.0 / m_actual);
    
    *result = total_combned_entropy;
}

void calculate_schedule_entropy_vertical(Processor *processor, double *result)
{
    SimulationData *sim_data = processor->simulation;
    double total_entropy = 0;
    int **t_data = sim_data->timeslots;
    int hyperperiod = sim_data->hyperperiod;
    int num_periods = sim_data->time / hyperperiod;
    int num_tasks = sim_data->num_tasks + processor->m;

    for (int i = 0; i < hyperperiod; i++)   // for every timeslot
    {
        double entropy = 0;
        for (int j = 0; j < num_tasks; j++)
        {
            int task_instances = 0;
            for (int k = 0; k < processor->m; k++)
            {
                task_instances += t_data[k][i * num_tasks + j];
            }
            double p = (double)task_instances / (num_periods);
            if (p > 0)
                entropy -= p * log2(p);
        }
        total_entropy += entropy;
    }

    total_entropy /= processor->m;

    *result = total_entropy;
}

void calculate_attack_data(TaskGroup* all_tasks, AttackData* attack_data, double *result) {
    int num_tasks = all_tasks->num_tasks;

    Task **tasks = all_tasks->tasks;

    double current_max_p_a = 0;
    double task_max_p_a[num_tasks];

    double current_max_p_p = 0;
    double task_max_p_p[num_tasks];

    double current_max_p_pinch = 0;
    double task_max_p_pinch[num_tasks];

    for (int i = 0; i < num_tasks; i++) {
        task_max_p_a[i] = 0;
        task_max_p_p[i] = 0;
        task_max_p_pinch[i] = 0;
        
        if (!tasks[i]->trusted)
            continue;

        AttackData *data = &attack_data[i];
        int num_instances = data->num_instances;
        double p_anterior_max = 0;
        double p_posterior_max = 0;
        double p_pincher_max = 0;

        for (int j = 0; j < num_tasks; j++) {
            // if (i == j)
            //     continue;

            if (tasks[j]->trusted)
                continue;

            if (data->anterior[j]) {
                double p_anterior = (double)data->anterior[j] / num_instances;
                if (p_anterior > p_anterior_max)
                    p_anterior_max = p_anterior;
                
                // printf("P(%d ant by %d) %f\n", tasks[i]->id, tasks[j]->id, p_anterior);
            }

            if (data->posterior[j]) {
                double p_posterior = (double)data->posterior[j] / num_instances;
                if (p_posterior > p_posterior_max)
                    p_posterior_max = p_posterior;
                    
                // printf("P(%d post by %d): %f\n", tasks[i]->id, tasks[j]->id, p_posterior);
            }

            if (data->pincher[j]){
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

    for (int i = 0; i < num_tasks; i++) {
        if (1 - task_max_p_a[i] < p_a_security)
            p_a_security = 1 - task_max_p_a[i];

        if (1 - task_max_p_p[i] < p_p_security)
            p_p_security = 1 - task_max_p_p[i];

        if (1 - task_max_p_pinch[i] < p_pinch_security)
            p_pinch_security = 1 - task_max_p_pinch[i];
    }

    result[0] = p_a_security;
    result[1] = p_p_security;
    result[2] = p_pinch_security;

    // printf("Max Pan: %f\n", current_max_p_a);
    // printf("Max Ppo: %f\n", current_max_p_p);
    // printf("Max Ppi: %f\n", current_max_p_pinch);
    // printf("\n");
}

void analyze_simulation(Processor *processor, double *result)
{
    int log_attack_data = processor->log_attack_data;
    int log_timeslot_data = processor->log_timeslot_data;

    if (log_attack_data)
    {
        double attack_data[3];
        calculate_attack_data(processor->all_tasks, processor->simulation->attack_data_h, attack_data);
        result[0] = attack_data[0];
        result[1] = attack_data[1];
        result[2] = attack_data[2];
        calculate_attack_data(processor->all_tasks, processor->simulation->attack_data, attack_data);
        result[3] = attack_data[0];
        result[4] = attack_data[1];
        result[5] = attack_data[2];
    }
        

    if (log_timeslot_data)
    {
        double entropy = 0;
        calculate_schedule_entropy_horizontal(processor, &entropy);
        result[6] = entropy;
        calculate_schedule_entropy_vertical(processor, &entropy);
        result[7] = entropy;
    }

    return;
}
