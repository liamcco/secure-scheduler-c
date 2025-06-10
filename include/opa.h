#ifndef OPA_H
#define OPA_H

#include "task.h"

int OPA_with_priority(Task **tasks, int num_tasks, int (*compare)(const void *, const void *));
int OPA_random(Task **tasks, int num_tasks);

#endif // OPA_H
