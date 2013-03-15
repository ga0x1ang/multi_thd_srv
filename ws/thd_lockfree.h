#ifndef THD_LOCKFREE_H
#define THD_LOCKFREE_H

#include <pthread.h>

void lockfree_run(int, int, int, pthread_t*, pthread_t*);

#endif
