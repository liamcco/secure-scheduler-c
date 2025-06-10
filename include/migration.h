#ifndef MIGRATION_H
#define MIGRATION_H

#include "processor.h"

void random_migration(Processor *processor);
void debug_partition(Partition *tasks);
void debug_scheduler(Scheduler *scheduler);

#endif // MIGRATION_H

