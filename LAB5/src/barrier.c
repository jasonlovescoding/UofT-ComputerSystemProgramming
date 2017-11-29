#include "barrier.h"

void barrier_wait(Barrier *barrier) {
	pthread_mutex_lock(barrier->lock);
	barrier->arrived++;
	if (barrier->arrived < barrier->bound) {
		pthread_cond_wait(barrier->cond, barrier->lock);	
	} else {
		pthread_cond_broadcast(barrier->cond);	
		barrier->arrived = 0;
	}
	pthread_mutex_unlock(barrier->lock);
}
