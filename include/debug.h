#ifndef DEBUG_H
#define DEBUG_H

#include "scheduler.h"
#include "partition_algorithms.h"

void debug_partition(Partition *tasks);
void debug_scheduler(Scheduler *scheduler);

#endif // DEBUG_H
