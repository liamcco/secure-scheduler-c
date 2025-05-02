#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>

#include "scheduler.h"
#include "task.h"
#include "priority.h"

Scheduler *init_scheduler_ts(void)
{
    Scheduler *scheduler = (Scheduler *)malloc(sizeof(Scheduler));

    scheduler->compare = &RM;

    scheduler->idle_task = init_idle_task();
    scheduler->to_schedule = 0;
    scheduler->previous_task = NULL;
    scheduler->push_back = 0;
    scheduler->risat_budget = 0;
    scheduler->adjust_budget = 0;

    scheduler->schedule_task = &schedule_task_ts;
    scheduler->task_completed = &task_completed_ts;
    scheduler->task_arrived = &task_arrived_ts;
    scheduler->time_step_scheduler = &time_step_scheduler_ts;
    scheduler->load_tasks_scheduler = &load_tasks_scheduler_ts;

    return scheduler;
}

Scheduler *init_scheduler_ts_custom(int (*comp)(const void *, const void *))
{
    Scheduler *scheduler = init_scheduler_ts();
    scheduler->compare = comp;
    return scheduler;
}

void load_tasks_scheduler_ts(Scheduler *scheduler, Task **tasks, int num_tasks)
{
    scheduler->tasks = tasks;
    scheduler->num_tasks = num_tasks;
    scheduler->idle_task->c_idx = num_tasks;
    scheduler->to_schedule = 0;

    if (scheduler->compare)
        prioritize(tasks, num_tasks, scheduler->compare);

    for (int i = 0; i < num_tasks; i++)
    {
        Task *task = tasks[i];

        task->c_idx = i;
        if (scheduler->risat_budget)
            task->maximum_inversion_budget = worst_case_maximum_inversion_budget_risat(task, tasks);
        else
            task->maximum_inversion_budget = worst_case_maximum_inversion_budget(task, tasks);
        task->remaining_inversion_budget = task->maximum_inversion_budget;
    }
}

void load_tasks_scheduler_mid(Scheduler *scheduler, Task **tasks, int num_tasks)
{
    scheduler->tasks = tasks;
    scheduler->num_tasks = num_tasks;
    scheduler->idle_task->c_idx = num_tasks;
    scheduler->to_schedule = 0;

    if (scheduler->compare)
        prioritize(tasks, num_tasks, scheduler->compare);

    for (int i = 0; i < num_tasks; i++)
    {
        Task *task = tasks[i];

        task->c_idx = i;
        if (scheduler->risat_budget)
            task->maximum_inversion_budget = worst_case_maximum_inversion_budget_risat(task, tasks);
        else
            task->maximum_inversion_budget = worst_case_maximum_inversion_budget(task, tasks);
        task->remaining_inversion_budget = 0;
    }
}

Task *schedule_task_ts(Scheduler *scheduler)
{
    // Check if it's time to schedule new task
    if (scheduler->to_schedule == 0)
    {
        scheduler->previous_task = pick_task(scheduler);
    }

    return scheduler->previous_task;
}

int pick_nonpushable_task(Task **ready_tasks, int tasks_to_consider, int consider_idle_task)
{
    int non_pushable_indices[tasks_to_consider];
    int count = 0;

    for (int i = 0; i < tasks_to_consider; i++) {
        if (!(ready_tasks[i]->remaining_execution_time == 1)) {
            non_pushable_indices[count++] = i;
        }
    }

    if (count == 0) return tasks_to_consider - 1 + consider_idle_task;  // Optional: handle case when no valid tasks

    int rand_idx = rand() % count;
    return non_pushable_indices[rand_idx];
}

// Implemets TaskShuffler
Task *pick_task(Scheduler *scheduler)
{
    // Tasks ready for execution
    Task *ready_tasks[scheduler->num_tasks];
    int num_ready_tasks = 0;
    for (int i = 0; i < scheduler->num_tasks; i++)
    {
        if (is_ready(scheduler->tasks[i]))
        {
            ready_tasks[num_ready_tasks] = scheduler->tasks[i];
            num_ready_tasks++;
        }
    }

    // If there are no tasks ready, keep idling, indefinetly
    if (num_ready_tasks == 0)
    {
        scheduler->to_schedule = -1;
        return scheduler->idle_task;
    }

    // Select the first task
    Task *selection = ready_tasks[0];

    // Can we afford to select any other task?
    if (selection->remaining_inversion_budget <= 0)
    {
        scheduler->to_schedule = selection->remaining_execution_time;
        if (scheduler->push_back && selection->remaining_execution_time > 1)
        {
            scheduler->to_schedule--;
        }
        return selection;
    }

    // Step 1: Find the set of tasks that can be selected
    int tasks_to_consider = 1;
    int m1 = minimum_inversion_priority(selection->c_idx, scheduler->tasks, scheduler->num_tasks); // Minimum inversion priority

    int consider_idle_task = 1;
    if (m1 < scheduler->num_tasks)
    {
        consider_idle_task = 0;
    }

    for (int i = 1; i < num_ready_tasks; i++)
    {
        if (ready_tasks[i]->c_idx <= m1)
        {
            tasks_to_consider++;
        }
        if (ready_tasks[i]->remaining_inversion_budget <= 0)
        {
            consider_idle_task = 0;
            break;
        }
    }

    // Step 2: Random selection
    int idx = rand() % (tasks_to_consider + consider_idle_task);
    if (scheduler->push_back)
        idx = pick_nonpushable_task(ready_tasks, tasks_to_consider, consider_idle_task);

    if (idx < tasks_to_consider)
    {
        selection = ready_tasks[idx];
    }
    else
    {
        selection = scheduler->idle_task;
    }

    // Step 3: if lower priority task is selected, schedule decision
    if (idx == 0)
    {
        scheduler->to_schedule = rand() % (selection->remaining_execution_time) + 1;
    }
    else if (idx == tasks_to_consider)
    {
        scheduler->to_schedule = next_schedule_decision_to_be_made_idle(ready_tasks, num_ready_tasks);
    }
    else
    {
        scheduler->to_schedule = next_schedule_decision_to_be_made(ready_tasks, idx);
    }

    if (scheduler->push_back && selection->remaining_execution_time == scheduler->to_schedule && scheduler->to_schedule > 1)
    {
        scheduler->to_schedule--;
    }

    return selection;
}

void decrement_task_budgets(Scheduler *scheduler)
{
    for (int i = 0; i < scheduler->previous_task->c_idx; i++)
    {
        if (is_ready(scheduler->tasks[i]))
        {
            scheduler->tasks[i]->remaining_inversion_budget--;
        }
    }

}

void task_completed_ts(Scheduler *scheduler, __attribute__((unused)) Task *task)
{
    scheduler->to_schedule = 0;
}

void task_arrived_ts(Scheduler *scheduler, Task *task)
{
    scheduler->to_schedule = 0;
    task->remaining_inversion_budget = task->maximum_inversion_budget;

    if (scheduler->adjust_budget) {
        task->remaining_inversion_budget += task->max_jitter - (task->period - task->time_until_next_period);
    }
        

    
    
}

void time_step_scheduler_ts(Scheduler *scheduler)
{
    // Decrement task budget after we now wich task is going to be executed
    decrement_task_budgets(scheduler);
    scheduler->to_schedule--;
}

int next_schedule_decision_to_be_made(Task **ready_tasks, int idx)
{
    int max_execution_time = ready_tasks[idx]->remaining_execution_time;
    for (int i = 0; i < idx; i++)
    {
        max_execution_time = MIN(max_execution_time, ready_tasks[i]->remaining_inversion_budget);
    }
    return rand() % (max_execution_time - 1 + 1) + 1;
}

int next_schedule_decision_to_be_made_idle(Task **ready_tasks, int num_ready_tasks)
{
    int max_execution_time = ready_tasks[0]->remaining_execution_time;
    for (int i = 0; i < num_ready_tasks; i++)
    {
        max_execution_time = MIN(max_execution_time, ready_tasks[i]->remaining_inversion_budget);
    }
    return rand() % (max_execution_time) + 1;
}

int minimum_inversion_priority(int c_idx, Task **tasks, int num_tasks)
{
    for (int i = c_idx; i < num_tasks; i++)
    {
        if (tasks[i]->maximum_inversion_budget < 0)
        {
            return tasks[i]->c_idx;
        }
    }

    return INT_MAX;
}

int worst_case_maximum_inversion_budget(Task *task_i, Task **tasks)
{
    return task_i->deadline - (task_i->duration + task_i->max_jitter + upper_bound_interference_from_hp(task_i, tasks));
}

int worst_case_maximum_inversion_budget_risat(Task *task_i, Task **tasks)
{
    return task_i->deadline - (task_i->duration + task_i->max_jitter + upper_bound_interference_from_hp_risat(task_i, tasks));
}

int upper_bound_interference_from_hp(Task *task_i, Task **tasks)
{
    int interference = 0;
    for (int i = 0; i < task_i->c_idx; i++)
    {
        Task *task = tasks[i];
        interference += (ceil((float)task_i->deadline / task->period) + 1) * task->duration;
    }
    return interference;
}

int upper_bound_interference_from_hp_risat(Task *task_k, Task **tasks)
{
    int interference = 0;
    for (int i = 0; i < task_k->c_idx; i++)
    {
        Task *task_i = tasks[i];
        if (task_i->maximum_inversion_budget <= 0) {
            interference += (ceil((float)task_k->deadline / task_i->period)) * task_i->duration;
            continue;
        }

        int Nk = floor((float)(task_k->deadline + task_i->maximum_inversion_budget) / (float)task_i->period);
        interference += Nk * task_i->duration + MIN(task_i->duration, (task_k->deadline + task_i->maximum_inversion_budget - Nk * task_i->period));
    }
    return interference;
}
