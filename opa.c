#include <stdlib.h>
#include <stdio.h>
#include "task.h"
#include "feasibility.h"

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
        // printf("About to assign task %d\n", assigned + 1);
        Task *candidates[num_tasks - assigned];
        int num_candidates = 0;

        // Try each task as the lowest priority
        for (int i = 0; i < num_tasks - assigned; i++)
        {
            Task *candidate = remaining_tasks[i];

            // Create a temporary set excluding the candidate task
            Task *subset[num_tasks - assigned - 1];
            int k = 0;
            for (int j = 0; j < num_tasks - assigned; j++)
            {
                if (remaining_tasks[j] != candidate)
                {
                    subset[k++] = remaining_tasks[j];
                }
            }

            // Check if candidate is schedulable with the remaining tasks
            if (response_time(candidate, subset, num_tasks - assigned - 1))
            {
                candidates[num_candidates++] = candidate;
            }
        }

        if (num_candidates == 0)
        {
            free(remaining_tasks);
            // printf("No feasible priority assignment found\n");
            return 0; // No feasible priority assignment found
        }

        // Sort candidates using the provided comparison function
        qsort(candidates, num_candidates, sizeof(Task *), compare);

        // Assign the lowest priority to the best candidate
        tasks[num_tasks - 1 - assigned] = candidates[0];

        // Remove the assigned task from the remaining set
        int j = 0;
        for (int i = 0; i < num_tasks - assigned; i++)
        {
            if (remaining_tasks[i] != candidates[0])
            {
                remaining_tasks[j++] = remaining_tasks[i];
            }
        }
        // printf("Assigned task %d\n", tasks[assigned]->id);
    }

    free(remaining_tasks);
    return 1;
}
