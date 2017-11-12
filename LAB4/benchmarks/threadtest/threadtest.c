////////////////////////////////////////////////////////////////////
//
// Hoard: A Fast, Scalable, and Memory-Efficient Allocator
//        for Shared-Memory Multiprocessors
// Contact author: Emery Berger, http://www.cs.utexas.edu/users/emery
//
// Copyright (c) 1998-2000, The University of Texas at Austin.
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
 * @file threadtest.c
 *
 * This program does nothing but generate a number of kernel threads
 * that allocate and free memory, with a variable
 * amount of "work" (i.e. cycle wasting) in between.
*/

#ifndef _REENTRANT
#define _REENTRANT
#endif


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "mm_thread.h"
#include "timer.h"
#include "malloc.h"
#include "memlib.h"

int niterations = 50;	// Default number of iterations.
int nobjects = 30000;   // Default number of objects.
int nthreads = 1;	// Default number of threads.
int work = 0;		// Default number of loop iterations.
int size = 1;

struct Foo {
  int x;
  int y;
};


extern void * worker (void *arg)
{
  int i, j;
  volatile int d;
  struct Foo ** a;
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
  int cpu = (int)arg; // cpu number will fit in an int, ignore warning
#pragma GCC diagnostic pop

  setCPU(cpu);

  a = (struct Foo **)mm_malloc( (nobjects / nthreads) * sizeof(struct Foo *));

  for (j = 0; j < niterations; j++) {

    // printf ("%d\n", j);
    for (i = 0; i < (nobjects / nthreads); i ++) {
      a[i] = (struct Foo *)mm_malloc(size*sizeof(struct Foo));
      for (d = 0; d < work; d++) {
	volatile int f = 1;
	f = f + f;
	f = f * f;
	f = f + f;
	f = f * f;
      }
      assert (a[i]);
    }
    
    for (i = 0; i < (nobjects / nthreads); i ++) {
      mm_free(a[i]);
      for (d = 0; d < work; d++) {
	volatile int f = 1;
	f = f + f;
	f = f * f;
	f = f + f;
	f = f * f;
      }
    }
  }

  mm_free(a);

  return NULL;
}


int main (int argc, char * argv[])
{
  
  if (argc >= 2) {
    nthreads = atoi(argv[1]);
  }

  if (argc >= 3) {
    niterations = atoi(argv[2]);
  }

  if (argc >= 4) {
    nobjects = atoi(argv[3]);
  }

  if (argc >= 5) {
    work = atoi(argv[4]);
  }

  if (argc >= 6) {
    size = atoi(argv[5]);
  }

  printf ("Running threadtest for %d threads, %d iterations, %d objects, %d work and %d size...\n", nthreads, niterations, nobjects, work, size);

  /* Call allocator-specific initialization function */
  mm_init();

  pthread_t *threads = (pthread_t *)mm_malloc(nthreads*sizeof(pthread_t));
  int numCPU = getNumProcessors();

  pthread_attr_t attr;
  initialize_pthread_attr(PTHREAD_CREATE_JOINABLE, SCHED_RR, -10, 
			  PTHREAD_EXPLICIT_SCHED, PTHREAD_SCOPE_SYSTEM, &attr);

  timer_start();

  int i;
  for (i = 0; i < nthreads; i++) {
	  pthread_create(&threads[i], &attr, &worker, (void *)((u_int64_t)(i+1)%numCPU));
  }

  for (i = 0; i < nthreads; i++) {
    pthread_join(threads[i], NULL);
  }

  double t = timer_stop();

  printf ("Time elapsed = %f seconds\n", t);
  printf ("Memory used = %ld bytes\n",mem_usage());

  mm_free(threads);

  return 0;
}
