#include <stdio.h>
#include <stdlib.h>

#include "opa.h"
#include "feasibility.h"

// Function to check if a task can be assigned the lowest priority
int OPA_with_priority(Task **tasks, int num_tasks, int (*compare)(const void *, const void *))
{
    if (num_tasks == 0)
        return 1;

    Task **remaining_tasks = malloc(num_tasks * sizeof(Task *));
    if (!remaining_tasks)
        return 0; // Memory allocation failure

    for (int i = 0; i < num_tasks; i++)
    {
        remaining_tasks[i] = tasks[i];
    }

    for (int assigned = 0; assigned < num_tasks; assigned++)
    {
        Task *candidates[num_tasks - assigned];
        int num_candidates = 0;

        // Find all tasks that can be assigned the lowest remaining priority
        for (int i = 0; i < num_tasks - assigned; i++)
        {
            if (response_time(remaining_tasks[i], remaining_tasks, num_tasks - assigned))
            {
                candidates[num_candidates++] = remaining_tasks[i];
            }
        }

        if (num_candidates == 0)
        {
            free(remaining_tasks);
            return 0; // No feasible priority assignment found
        }

        // Sort candidates using the provided comparison function
        qsort(candidates, num_candidates, sizeof(Task *), compare);

        // Assign the lowest priority to the best candidate
        tasks[assigned] = candidates[0];

        // Remove the assigned task from the remaining set
        int j = 0;
        for (int i = 0; i < num_tasks - assigned; i++)
        {
            if (remaining_tasks[i] != candidates[0])
            {
                remaining_tasks[j++] = remaining_tasks[i];
            }
        }
    }

    free(remaining_tasks);
    return 1;
}
