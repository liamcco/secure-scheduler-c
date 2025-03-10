#ifndef TASKSET_H
#define TASKSET_H

#include "task.h"

struct Divisors
{
    int *divisors;
    int num_divisors;
};

struct Divisors *find_divisors(int n);
double *randfixedsum(int n, double u, double a, double b);
int compare_doubles(const void *a, const void *b);
Task **generate_task_set(int n, double U, int hyper_period, int lowest_duration, int highest_duration);

#endif // TASKSET_H
