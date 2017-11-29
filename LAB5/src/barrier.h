#ifndef _barrier_h
#define _barrier_h
#include <pthread.h>

typedef struct tagBarrier {
	int arrived;
	int bound;
	pthread_mutex_t *lock;
	pthread_cond_t *cond;
} Barrier;

void barrier_wait(Barrier *barrier);

#endif
