#include "barrier.h"

// the barrier wait triggers a conditional wait until
// the number of arrival upon this barrier reaches its bound
void barrier_wait(Barrier *barrier) {
	// protect cond as critical section
	pthread_mutex_lock(barrier->lock);
	barrier->arrived++;
	if (barrier->arrived < barrier->bound) {
		// not enough arrivals yet
		pthread_cond_wait(barrier->cond, barrier->lock);	
	} else {
		// enough arrival reaches, awaken all threads
		pthread_cond_broadcast(barrier->cond);	
		// and clear the arrival for the next round
		barrier->arrived = 0;
	}
	pthread_mutex_unlock(barrier->lock);
}
