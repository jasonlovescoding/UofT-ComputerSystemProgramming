#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdio.h>
#include <stddef.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "tsc.h"



int comp(const void *elem1, const void *elem2) {
	u_int64_t a = *((u_int64_t *)elem1);
	u_int64_t b = *((u_int64_t *)elem2);
	if (a > b) return 1;
	if (a < b) return -1;
	return 0;
}

static double getFrequency (void) {
	static double freq = 0.0;
	struct timespec req;
	int err;
	int i,trials = 20;
	u_int64_t samp[20];
        u_int64_t min = INT_MAX;

	// Use previously computed value, if available
	if (freq != 0.0) {
		return freq;
	}

	// Compute CPU frequency directly

	// Wait for approximately one second.
	req.tv_sec = 1;
	req.tv_nsec = 0;

	for (i=0; i < trials; i++) {
		start_counter();
		err = nanosleep (&req,NULL);
		samp[i] = get_counter();
	}

	qsort(samp, sizeof(samp)/sizeof(*samp), sizeof(*samp), comp);
	// Assumption: CPU frequency scaling will step at least 100MHz at a
	// time. (On wolf, for example the step between any two available
	// frequencies is 300 or 400 MHz. See 
	// /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies)
	// This means as long as the difference between max and min sample is
	// less than 100,000,000 cycles, frequency scaling has not interfered
	// with the measurement.  If the range is larger than 100MHz, either
	// the system is heavily loaded (lots of extra delay after nanosleep)
	// or frequency scaling is interfering. 
	if (samp[trials-1] - samp[0] > 1e8) {
		printf("Range too large, min %lu MHz, max %lu MHz.\n",
		       samp[0]/1000000,samp[trials-1]/1000000);		
	}

	freq = (double)samp[0];

	return freq;
}

void timer_start (void) {
  start_counter();
}

double timer_stop (void) {
  u_int64_t elapsed = get_counter();
  double freq = getFrequency ();
  return elapsed / freq;
}

#endif /* _TIMER_H */
