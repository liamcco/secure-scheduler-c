#ifndef OPA_H
#define OPA_H

#include "task.h"

int OPA(Task **tasks, int num_tasks, int (*compare)(const void *, const void *));

#endif // OPA_H
