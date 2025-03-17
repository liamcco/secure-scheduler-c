#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include "task.h"
#include "taskset.h"

Task **generate_task_set(int n, double U, int hyper_period, int lowest_duration, int highest_duration)
{
    // Generates a task set with a given:
    // number of tasks, n
    // total utilization, U
    // hyper period, hyper_period (optional),
    // range of execution time, duration_range (optional)

    // Generate task utilizations
    // utils = TaskSet.u_uni_fast(n, U)
    double *utils = randfixedsum(n, U, 0, 1);

    // Get valid periods (must be a divisor of period_limit)
    struct Divisors *divisors = find_divisors(hyper_period);

    Task **task_set = malloc(n * sizeof(Task *));

    for (int i = 0; i < n; i++)
    {
        // Random time between lowest_duration and highest_duration
        int execution_time = rand() % (highest_duration - lowest_duration + 1) + lowest_duration;

        // Calculate the required period to satisfy U = C / T => T = C / U
        int period_optimal = round((double)execution_time / utils[i]);

        // Find the closest valid period (must be a divisor of period_limit)
        // period = min(valid_periods, key = lambda p : abs(p - period));

        // Find the closest valid period (must be a divisor of period_limit) in C
        int current_best_period = divisors->divisors[divisors->num_divisors - 1];
        for (int j = 0; j < divisors->num_divisors; j++)
        {
            if (abs(divisors->divisors[j] - period_optimal) < abs(current_best_period - period_optimal) && (double)execution_time / (double)divisors->divisors[j] < 1)
            {
                current_best_period = divisors->divisors[j];
            }
        }

        Task *task = init_task(current_best_period, execution_time);
        task_set[i] = task;
    }

    free(utils);
    free(divisors->divisors);
    free(divisors);

    return task_set;
}

// Function to find divisors and return them in a dynamically allocated array
struct Divisors *find_divisors(int n)
{
    int capacity = 2 * (int)sqrt(n); // Rough upper limit for memory allocation
    int *divisors = (int *)malloc(capacity * sizeof(int));

    int num_divisors = 0;

    // Find divisors up to sqrt(n)
    for (int i = 1; i * i <= n; i++)
    {
        if (n % i == 0)
        {
            divisors[num_divisors++] = i; // Add divisor

            if (i != n / i)
            { // Avoid adding the square root twice
                divisors[num_divisors++] = n / i;
            }
        }
    }

    struct Divisors *divisors_container = (struct Divisors *)malloc(sizeof(struct Divisors));
    divisors_container->divisors = divisors;
    divisors_container->num_divisors = num_divisors;

    return divisors_container;
}

// Function to compare two doubles for qsort (for permutations later)
int compare_doubles(const void *a, const void *b)
{
    double diff = *(double *)a - *(double *)b;
    return (diff > 0) - (diff < 0);
}

// Core function for randfixedsum
double *randfixedsum(int n, double u, double a, double b)
{
    if (n <= 0 || (u < n * a) || (u > n * b) || (a >= b))
    {
        printf("Invalid input parameters.\n");
        return NULL;
    }

    // Rescale u to [0,1] interval
    u = (u - n * a) / (b - a);

    // Initialize variables
    int k = (int)fmin(fmax(floor(u), 0), n - 1);
    u = fmax(fmin(u, k + 1), k);

    double *u1 = malloc(n * sizeof(double));
    double *u2 = malloc(n * sizeof(double));
    for (int i = 0; i < n; i++)
    {
        u1[i] = u - (k - i);
        u2[i] = (k + n - i) - u;
    }

    double **w = malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++)
    {
        w[i] = calloc(n + 1, sizeof(double));
    }
    w[0][1] = DBL_MAX;

    double **t = malloc((n - 1) * sizeof(double *));
    for (int i = 0; i < n - 1; i++)
    {
        t[i] = calloc(n, sizeof(double));
    }

    // Build transition probabilities
    double tiny = pow(2, -1074); // Smallest positive double
    for (int i = 1; i < n; i++)
    {
        for (int j = 1; j <= i + 1; j++)
        {
            double tmp1 = w[i - 1][j] * u1[j - 1] / (i + 1);
            double tmp2 = w[i - 1][j - 1] * u2[n - i] / (i + 1);
            w[i][j] = tmp1 + tmp2;

            double tmp3 = w[i][j] + tiny;
            t[i - 1][j - 1] = (tmp2 / tmp3) * (u2[n - i] > u1[j - 1]) + (1.0 - tmp1 / tmp3) * (u2[n - i] <= u1[j - 1]);
        }
    }

    // Generate random numbers
    double *x = calloc(n, sizeof(double));
    double *rt = malloc((n - 1) * sizeof(double));
    double *ru = malloc((n - 1) * sizeof(double));

    for (int i = 0; i < n - 1; i++)
    {
        rt[i] = (double)rand() / RAND_MAX;
        ru[i] = pow((double)rand() / RAND_MAX, 1.0 / (i + 1));
    }

    int j = k + 1;
    double u_current = u;
    double sum = 0.0, prod = 1.0;

    for (int i = n - 1; i > 0; i--)
    {
        int e = rt[n - i - 1] <= t[i - 1][j - 1];
        double ux = ru[n - i - 1];

        sum += (1.0 - ux) * prod * u_current / (i + 1);
        prod *= ux;
        x[n - i - 1] = sum + prod * e;
        u_current -= e;
        j -= e;
    }
    x[n - 1] = sum + prod * u_current;

    // Random permutation of x
    for (int i = 0; i < n; i++)
    {
        double r = (double)rand() / RAND_MAX;
        x[i] += r * tiny;
    }
    qsort(x, n, sizeof(double), compare_doubles);

    // Rescale back to [a, b]
    for (int i = 0; i < n; i++)
    {
        x[i] = x[i] * (b - a) + a;
    }

    // Cleanup
    free(u1);
    free(u2);
    for (int i = 0; i < n; i++)
        free(w[i]);
    free(w);
    for (int i = 0; i < n - 1; i++)
        free(t[i]);
    free(t);
    free(rt);
    free(ru);

    return x;
}
