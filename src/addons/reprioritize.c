#include <stdlib.h>
#include "reprioritze.h"
#include "opa.h"

void reprioritze(Processor *processor)
{
    for (int i = 0; i < processor->m; i++)
    {
        TaskGroup *group = processor->tasks->task_groups[i];
        int num_tasks = group->num_tasks;
        OPA_random(group->tasks, num_tasks);
        processor->cores[i]->scheduler->compare = NULL;
        load_tasks_core_mid(processor->cores[i], group);
    }
}
