/////////////////////////////////////////////////////////////////////
//
// Hoard: A Fast, Scalable, and Memory-Efficient Allocator
//        for Shared-Memory Multiprocessors
// Contact author: Emery Berger, http://www.cs.umass.edu/~emery
//
// Copyright (c) 1998-2003, The University of Texas at Austin.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as
// published by the Free Software Foundation, http://www.fsf.org.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file cache-scratch.c
 *
 * cache-scratch is a benchmark that exercises a heap's cache-locality.
 * An allocator that allows multiple threads to re-use the same small
 * object (possibly all in one cache-line) will scale poorly, while
 * an allocator like Hoard will exhibit near-linear scaling.
 *
 * Try the following (on a P-processor machine):
 *
 *  cache-scratch 1 1000 1 1000000
 *  cache-scratch P 1000 1 1000000
 *
 *  cache-scratch-hoard 1 1000 1 1000000
 *  cache-scratch-hoard P 1000 1 1000000
 *
 *  The ideal is a P-fold speedup.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "mm_thread.h"
#include "memlib.h"
#include "timer.h"
#include "malloc.h"

// This struct just holds arguments to each thread.
struct workerArg {
  char * _object;
  int _objSize;
  int _iterations;
  int _repetitions;
  int _cpu;
};


extern void * worker (void * arg)
{
  // free the object we were given.
  // Then, repeatedly do the following:
  //   malloc a given-sized object,
  //   repeatedly write on it,
  //   then free it.

  int i, j, k; /* Loop control variables */

  struct workerArg * w = (struct workerArg *) arg;
  setCPU(w->_cpu);
  
  mm_free(w->_object);
  for (i = 0; i < w->_iterations; i++) {
    // Allocate the object.
    char * obj = (char *)mm_malloc(w->_objSize);
    // Write into it a bunch of times.
    for (j = 0; j < w->_repetitions; j++) {
      for (k = 0; k < w->_objSize; k++) {
	obj[k] = (char) k;
	volatile char ch = obj[k];
	ch++;
      }
    }
    // Free the object.
    mm_free(obj);
  }
  mm_free(w);

  return NULL;
}


int main (int argc, char * argv[])
{
  int nthreads;
  int iterations;
  int objSize;
  int repetitions;

  if (argc > 4) {
    nthreads = atoi(argv[1]);
    iterations = atoi(argv[2]);
    objSize = atoi(argv[3]);
    repetitions = atoi(argv[4]);
  } else {
    fprintf (stderr, "Usage: %s nthreads iterations objSize repetitions\n", argv[0]);
    return 1;
  }

  pthread_t threads[nthreads];
  int numCPU = getNumProcessors();

  int i;

  double freq = getFrequency();
  printf("Got freq of %lf MHz\n",freq/1e6);
  /* Call allocator-specific initialization function */
  mm_init();

  // Allocate nthreads objects and distribute them among the threads.
  char ** objs = (char **)mm_malloc(nthreads * sizeof(char *));
  for (i = 0; i < nthreads; i++) {
    objs[i] = (char *)mm_malloc(objSize);
  }
  
  pthread_attr_t attr;
  initialize_pthread_attr(PTHREAD_CREATE_JOINABLE, SCHED_RR, -10, PTHREAD_EXPLICIT_SCHED, 
			  PTHREAD_SCOPE_SYSTEM, &attr);

  timer_start();

  for (i = 0; i < nthreads; i++) {
    struct workerArg * w = (struct workerArg *)mm_malloc(sizeof(struct workerArg));
    w->_object = objs[i];
    w->_objSize = objSize;
    w->_repetitions = repetitions / nthreads;
    w->_iterations = iterations;
    w->_cpu = (i+1)%numCPU;
    pthread_create(&threads[i], &attr, &worker, (void *)w);
  }

  for (i = 0; i < nthreads; i++) {
    pthread_join(threads[i], NULL);
  }

  double t = timer_stop();

  mm_free(objs);

  printf ("Time elapsed = %f seconds\n", t);
  printf ("Memory used = %ld bytes\n",mem_usage());
  return 0;
}
