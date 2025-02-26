#ifndef PRIORITY_H
#define PRIORITY_H

#include "task.h"

void prioritize(Task **tasks, int num_tasks, int (*compare)(const void *, const void *));
int RM(const void *elem1, const void *elem2);
int RRM(const void *elem1, const void *elem2);
int DM(const void *elem1, const void *elem2);
int RDM(const void *elem1, const void *elem2);
int IU(const void *elem1, const void *elem2);
int DU(const void *elem1, const void *elem2);
int SM(const void *elem1, const void *elem2);
int RSM(const void *elem1, const void *elem2);
#endif // PRIORITY_H