# Here is how a basic program works:

1. Task set generatiion:
`Task **tasks = generate_task_set(n, U, hyper_period, 1, 50);`
(1, 50 is the range of allowed execution times)
DO NOT forget to call free_tasks(tasks, n) when you are done with a task set.

2. Create a processor
`Processor *processor = init_processor_custom(m, &init_scheduler_ts);`
m = num of cores,
init_scheduler_ts is initializer for which scheduler you want each core to run.

You may use: (from scheduler.h)

init_scheduler_fp(void)
^ fixed priority RM

init_scheduler_fp_custom(int (*comp)) 
^ fixed priority custom priority

init_scheduler_ts(void) 
^ task shuffler RM

init_scheduler_ts_custom(int (*comp))
^ task shuffler custom priortiy

To create a fixed priority SM scheduler you must provide the processor init with a Scheduler *init(void):

`Scheduler *init_SM_fp(void) {
    Scheduler sm_scheduler = init_scheduler_fp_custom(&SM);
    return sm_scheduler
}

Processor *processor = init_processor_custom(m, &init_SM_fp)`

Basic ordering can be found in priority.h

DO NOT forget to call free_processor(processor) when you are done with a processor.

3. Load the tasks

Load tasks with:
int feasible = load_tasks(processor, tasks, n, &bf);
where &bf is the address to any partition algorithm you want to use.

Observe: When loading tasks, the return value of the function will tell you if the task set is feasible, If you attempt to continue and run the processor, the program may crash

A few common ones that already exist are:
ff, nf, bf, w.
These all automatically run RTA with RM to determine if it is feasiible to assign a task to a core. If you have provided a task set to big, not all tasks will be assigned, the processor will notice, and return 0 from the load_tasks function.

You may use ff_nosort if you want to the first task in the task array to have highest priority.

partition_from_allocation can be used to try a custom partition, and requires a special load_tasks_function:
int feasible = load_tasks_from_allocation(processor, tasks, n, allocation);
Allocation is an array of length n with 1-indexed indexed for which core to assign the tasks to
example: allocation = { 1, 2, 3, 1 }
will result in:
1: Task 1, Task 4
2: Task 2
3: Task 3

4. Setup analysis
To capture timeslot data:
processor->log_timeslot_data = 1;

To capture anterior data:
processor->log_attack_data = 1;

Provide custom analysis:
processor->analyze = &analyze_simulation;

Currently, analyze_simulation calculates either entropy or anterior/posterior scores depending on which flags are set. Remember to always set the unused ones to 0.

As of now, you can provide the processor with a pointer to a double array to retrieve as many values as you desire.
double result[3];
for (i=0; i<3; i++) {
    result[i] = 0;
}

5. Run the processor

success = run(processor, hyper_period * 1000, result);

If simulation did not succeed, run will return 0.

If success is 1, your data will be ready in result.

6. Print data

In order to gather data from all threads, I print the results on one line, with all data required for the data point:
printf("U=%.2f,", actual_U);
printf("CP1=%.3f,", ff / max);
printf("Med=%.3f,", median / max);
printf("Min=%.3f,", min / max);
printf("Avg=%.3f", avg / max);
printf("\n");

7. Big simulations. Create a for loop and vary any parameter and print the result to the console. Remember to free all resources every iteraton of the for loop.

8. Run in cluster:
make clean && make

sbatch job.sh

job.sh:
#!/bin/bash
#SBATCH -p vera
#SBATCH -A C3SE2025-1-14
#SBATCH -n 60
#SBATCH -t 0-01:50:00

srun secure-scheduler-c/program (or wherever your output program is)

9. Plot data
Your program output will result in slurmXXXXXX.out,  which i just copy to a local file and write a Python script to plot the data


