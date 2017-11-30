/*****************************************************************************
Parallelization strategy: barrier parallelization
- We choose barrier parallelization to avoid the release and join of worker threads at every iteration.
  With a barrier the threads will wait for each other until all of them finished this iteration.
  This is necessary because each thread will rely on the correctness of its neighbor of boundary column,
  which is written by its neighbor worker.

Optimization: 1. avoid unnecessary reading; 2. loop peeling; 3. loop unrolling; 
1. 
- This is based on a observation on the sequential implementation: each pixel in the image is read for 9 times.
We instead use a lookup window to reduce this number to 3 (could be better but bounded by the limit of registers).
we choose to use a 3-by-3 window, and each time the window center slides to the next pixel, only a new 3-pixel column
will need to read.
2. 
- This is divided into 2 levels:
At column level, we choose to split the thread worker into 3 types.
if the worker deals with top part of the image, its window's starting column needs to be wrapped around ncols. This is hand-coded.
if the worker deals with middle part of the image, no wrap-around is needed.
if the worker deals with the bottom part of the image, its window's ending column needs to be wrapped around ncols. This is hand-coded.
By peeling this loop, we avoided any invoking of mod at column level.

At row level, we choose to split the iteration. Our first lookup window will be centered at the bottom of the row, so that
the only pixel that needs to be wrapped around will be treated outside the loop. So in the loop of rows, no wrap-around is needed.

Combined, we get rid of the function usage of 'mod'.
3. 
- At row level, we found that the sliding of the lookup window can be unrolled. 
After some testing we decided to unroll by 16.

*****************************************************************************/

/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <stdio.h>
#include <pthread.h>
#include "barrier.h"
#include <string.h>

/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/
/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])

// test machine is 8-core
#define NUM_THREADS 8

// just for generating reference solution..
#define ONETHREAD_MODE 0

// the arguments for worker threads 
typedef struct tagArg {
	char *outboard;
	char *inboard;
	int nrows;
	int ncols;
	int col_start;
	int col_end;
	int gens_max;
	Barrier *barrier;
} Arg;

// this macro controls the sliding of lookup window
#define UNROLL( __i) do {\
	nwe = we; \
	n = c; \
	we = swe; \
	c = s; \
	swe = BOARD(inboard, __i + 1, jwest) + BOARD(inboard, __i + 1, jeast); \
	s = BOARD(inboard, __i + 1, j); \
	BOARD(outboard, __i, j) = alivep(nwe + n + we + swe + s, c); \
} while(0)

// worker threads are divided into 3 classes: top, middle and bottom
// corresponding to the partition of image it deals with
// this is for avoiding the calling of mod function
void *top_worker(void *arg_p) {
	// read the args
	Arg *arg = (Arg *)arg_p;
	char *outboard = arg->outboard;
	char *inboard = arg->inboard;
	const int nrows = arg->nrows;
	const int ncols = arg->ncols;
	const int col_end = arg->col_end;
	const int LDA = nrows;
	const int gens_max = arg->gens_max;
	Barrier *barrier = arg->barrier;
	
	char nwe, n, we, c, swe, s;
	int jwest, jeast;
	int i, j;
	// per-thread sequential part
	for (int curgen = 0; curgen < gens_max; curgen++) {
		// for worker at top of this image, the starting column is 0, and the wrap-around is hand coded as such
		j = 0; {
			jwest = ncols - 1;
			jeast = 1;

			nwe = BOARD(inboard, nrows - 2, jwest) + BOARD(inboard, nrows - 2, jeast);
			n = BOARD(inboard, nrows - 2, j);

			we = BOARD(inboard, nrows - 1, jwest) + BOARD(inboard, nrows - 1, jeast);
			c = BOARD(inboard, nrows - 1, j);

			swe = BOARD(inboard, 0, jwest) + BOARD(inboard, 0, jeast);
			s = BOARD(inboard, 0, j);
			
			BOARD(outboard, nrows - 1, j) = alivep(nwe + n + we + swe + s, c); 

			for (i = 0; i < nrows - 16; i += 16) {
				UNROLL(i  );
				UNROLL(i+1);
				UNROLL(i+2);
				UNROLL(i+3);
				UNROLL(i+4);
				UNROLL(i+5);
				UNROLL(i+6);
				UNROLL(i+7);
				UNROLL(i+8);
				UNROLL(i+9);
				UNROLL(i+10);
				UNROLL(i+11);
				UNROLL(i+12);
				UNROLL(i+13);
				UNROLL(i+14);
				UNROLL(i+15);
			}
				UNROLL(i  );
				UNROLL(i+1);
				UNROLL(i+2);
				UNROLL(i+3);
				UNROLL(i+4);
				UNROLL(i+5);
				UNROLL(i+6);
				UNROLL(i+7);
				UNROLL(i+8);
				UNROLL(i+9);
				UNROLL(i+10);
				UNROLL(i+11);
				UNROLL(i+12);
				UNROLL(i+13);
				UNROLL(i+14);
		}	
		// the image is column-major in memory, so is the loop		
		for (j++; j < col_end; j++) {
			jwest = j-1;
			jeast = j+1;

			nwe = BOARD(inboard, nrows - 2, jwest) + BOARD(inboard, nrows - 2, jeast);
			n = BOARD(inboard, nrows - 2, j);

			we = BOARD(inboard, nrows - 1, jwest) + BOARD(inboard, nrows - 1, jeast);
			c = BOARD(inboard, nrows - 1, j);

			swe = BOARD(inboard, 0, jwest) + BOARD(inboard, 0, jeast);
			s = BOARD(inboard, 0, j);
			
			BOARD(outboard, nrows - 1, j) = alivep(nwe + n + we + swe + s, c); 

			for (i = 0; i < nrows - 16; i += 16) {
				UNROLL(i  );
				UNROLL(i+1);
				UNROLL(i+2);
				UNROLL(i+3);
				UNROLL(i+4);
				UNROLL(i+5);
				UNROLL(i+6);
				UNROLL(i+7);
				UNROLL(i+8);
				UNROLL(i+9);
				UNROLL(i+10);
				UNROLL(i+11);
				UNROLL(i+12);
				UNROLL(i+13);
				UNROLL(i+14);
				UNROLL(i+15);
			}
				UNROLL(i  );
				UNROLL(i+1);
				UNROLL(i+2);
				UNROLL(i+3);
				UNROLL(i+4);
				UNROLL(i+5);
				UNROLL(i+6);
				UNROLL(i+7);
				UNROLL(i+8);
				UNROLL(i+9);
				UNROLL(i+10);
				UNROLL(i+11);
				UNROLL(i+12);
				UNROLL(i+13);
				UNROLL(i+14);
		}	
		SWAP_BOARDS(outboard, inboard);
		barrier_wait(barrier);
	}
	return NULL;
}

void *middle_worker(void *arg_p) {
	// read the args
	Arg *arg = (Arg *)arg_p;
	char *outboard = arg->outboard;
	char *inboard = arg->inboard;
	const int nrows = arg->nrows;
	const int col_start = arg->col_start;
	const int col_end = arg->col_end;
	const int LDA = nrows;
	const int gens_max = arg->gens_max;
	Barrier *barrier = arg->barrier;
	
	char nwe, n, we, c, swe, s;
	int jwest, jeast;
	int i, j;
	// per-thread sequential part
	for (int curgen = 0; curgen < gens_max; curgen++) {
		// the image is column-major in memory, so is the loop		
		// for workers at middle of the image, there is no need for wrap-around
		for (j = col_start; j < col_end; j++) {
			jwest = j-1;
			jeast = j+1;

			nwe = BOARD(inboard, nrows - 2, jwest) + BOARD(inboard, nrows - 2, jeast);
			n = BOARD(inboard, nrows - 2, j);

			we = BOARD(inboard, nrows - 1, jwest) + BOARD(inboard, nrows - 1, jeast);
			c = BOARD(inboard, nrows - 1, j);

			swe = BOARD(inboard, 0, jwest) + BOARD(inboard, 0, jeast);
			s = BOARD(inboard, 0, j);
			
			BOARD(outboard, nrows - 1, j) = alivep(nwe + n + we + swe + s, c); 

			for (i = 0; i < nrows - 16; i += 16) {
				UNROLL(i  );
				UNROLL(i+1);
				UNROLL(i+2);
				UNROLL(i+3);
				UNROLL(i+4);
				UNROLL(i+5);
				UNROLL(i+6);
				UNROLL(i+7);
				UNROLL(i+8);
				UNROLL(i+9);
				UNROLL(i+10);
				UNROLL(i+11);
				UNROLL(i+12);
				UNROLL(i+13);
				UNROLL(i+14);
				UNROLL(i+15);
			}
				UNROLL(i  );
				UNROLL(i+1);
				UNROLL(i+2);
				UNROLL(i+3);
				UNROLL(i+4);
				UNROLL(i+5);
				UNROLL(i+6);
				UNROLL(i+7);
				UNROLL(i+8);
				UNROLL(i+9);
				UNROLL(i+10);
				UNROLL(i+11);
				UNROLL(i+12);
				UNROLL(i+13);
				UNROLL(i+14);
		}	
		SWAP_BOARDS(outboard, inboard);
		barrier_wait(barrier);
	}
	return NULL;
}

void *bottom_worker(void *arg_p) {
	// read the args
	Arg *arg = (Arg *)arg_p;
	char *outboard = arg->outboard;
	char *inboard = arg->inboard;
	const int nrows = arg->nrows;
	const int col_start = arg->col_start;
	const int col_end = arg->col_end;
	const int LDA = nrows;
	const int gens_max = arg->gens_max;
	Barrier *barrier = arg->barrier;
	
	char nwe, n, we, c, swe, s;
	int jwest, jeast;
	int i, j;
	// per-thread sequential part
	for (int curgen = 0; curgen < gens_max; curgen++) {
		// the image is column-major in memory, so is the loop		
		for (j = col_start; j < col_end - 1; j++) {
			jwest = j-1;
			jeast = j+1;

			nwe = BOARD(inboard, nrows - 2, jwest) + BOARD(inboard, nrows - 2, jeast);
			n = BOARD(inboard, nrows - 2, j);

			we = BOARD(inboard, nrows - 1, jwest) + BOARD(inboard, nrows - 1, jeast);
			c = BOARD(inboard, nrows - 1, j);

			swe = BOARD(inboard, 0, jwest) + BOARD(inboard, 0, jeast);
			s = BOARD(inboard, 0, j);
			
			BOARD(outboard, nrows - 1, j) = alivep(nwe + n + we + swe + s, c); 

			for (i = 0; i < nrows - 16; i += 16) {
				UNROLL(i  );
				UNROLL(i+1);
				UNROLL(i+2);
				UNROLL(i+3);
				UNROLL(i+4);
				UNROLL(i+5);
				UNROLL(i+6);
				UNROLL(i+7);
				UNROLL(i+8);
				UNROLL(i+9);
				UNROLL(i+10);
				UNROLL(i+11);
				UNROLL(i+12);
				UNROLL(i+13);
				UNROLL(i+14);
				UNROLL(i+15);
			}
				UNROLL(i  );
				UNROLL(i+1);
				UNROLL(i+2);
				UNROLL(i+3);
				UNROLL(i+4);
				UNROLL(i+5);
				UNROLL(i+6);
				UNROLL(i+7);
				UNROLL(i+8);
				UNROLL(i+9);
				UNROLL(i+10);
				UNROLL(i+11);
				UNROLL(i+12);
				UNROLL(i+13);
				UNROLL(i+14);
		}
		// for worker at bottom of the image, the end column is the last column, and the wrap-around is hand-coded as such	
		{
			jwest = col_end - 2;
			jeast = 0;

			nwe = BOARD(inboard, nrows - 2, jwest) + BOARD(inboard, nrows - 2, jeast);
			n = BOARD(inboard, nrows - 2, j);

			we = BOARD(inboard, nrows - 1, jwest) + BOARD(inboard, nrows - 1, jeast);
			c = BOARD(inboard, nrows - 1, j);

			swe = BOARD(inboard, 0, jwest) + BOARD(inboard, 0, jeast);
			s = BOARD(inboard, 0, j);
			
			BOARD(outboard, nrows - 1, j) = alivep(nwe + n + we + swe + s, c); 

			for (i = 0; i < nrows - 16; i += 16) {
				UNROLL(i  );
				UNROLL(i+1);
				UNROLL(i+2);
				UNROLL(i+3);
				UNROLL(i+4);
				UNROLL(i+5);
				UNROLL(i+6);
				UNROLL(i+7);
				UNROLL(i+8);
				UNROLL(i+9);
				UNROLL(i+10);
				UNROLL(i+11);
				UNROLL(i+12);
				UNROLL(i+13);
				UNROLL(i+14);
				UNROLL(i+15);
			}
				UNROLL(i  );
				UNROLL(i+1);
				UNROLL(i+2);
				UNROLL(i+3);
				UNROLL(i+4);
				UNROLL(i+5);
				UNROLL(i+6);
				UNROLL(i+7);
				UNROLL(i+8);
				UNROLL(i+9);
				UNROLL(i+10);
				UNROLL(i+11);
				UNROLL(i+12);
				UNROLL(i+13);
				UNROLL(i+14);
		}	
		SWAP_BOARDS(outboard, inboard);
		barrier_wait(barrier);
	}
	return NULL;
}

/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
	if (nrows < 32  || ONETHREAD_MODE) {
		// no need to parallelize
		return sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);	
	} else if (nrows != ncols || nrows % 2 != 0 || nrows > 10000) {
		// gracefully exit...
		fprintf(stderr, "You should provide an (N,N) image where N is power of 2 and N<=10000\n");
		bzero(outboard, nrows * ncols);
		return outboard;
	}
	// components of barrier
	pthread_mutex_t lock;
	pthread_mutex_init(&lock, NULL);
	pthread_cond_t cond;
	pthread_cond_init(&cond, NULL);
	
	// the barrier should let threads pass only when all threads have finished this iteration because the boundaries are shared
	Barrier barrier;
	barrier.cond = &cond;
	barrier.lock = &lock;
	barrier.bound = NUM_THREADS;
	barrier.arrived = 0;
	pthread_t tid[NUM_THREADS];
	Arg arg[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS; i++) {
		// initialize workers' arguments
		arg[i].outboard = outboard;
		arg[i].inboard = inboard;
		arg[i].nrows = nrows;
		arg[i].ncols = ncols;
		arg[i].col_start = i * (ncols / NUM_THREADS); // assumption: ncols is pow of 2
		arg[i].col_end = (i + 1) * (ncols / NUM_THREADS);
		arg[i].gens_max = gens_max;
		arg[i].barrier = &barrier;
	}
	// create threads at top
	pthread_create(&tid[0], NULL, top_worker, (void *)&arg[0]);	
	for (int i = 1; i < NUM_THREADS - 1; i++) {
		// create threads at middle
		pthread_create(&tid[i], NULL, middle_worker, (void *)&arg[i]);	
	}
	// create threads at bottom
	pthread_create(&tid[NUM_THREADS - 1], NULL, bottom_worker, (void *)&arg[NUM_THREADS - 1]);	

	for (int i = 0; i < NUM_THREADS; i++) {
		// wait until all threads finished
    	pthread_join(tid[i], NULL);
  	}
	
	return inboard;
}

