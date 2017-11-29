/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <stdio.h>
#include <pthread.h>
#include "barrier.h"

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

#define ONETHREAD_MODE 0
 
typedef struct tagArg {
	char *outboard;
	char *inboard;
	int nrows;
	int ncols;
	int col_start;
	int col_end;
	int LDA;
	int gens_max;
	Barrier *barrier;
} Arg;

#define UNROLL( __i) do {\
	nwe = we; n = c; \
	we = swe; c = s; \
	swe = BOARD(inboard, __i + 1, jwest) + BOARD(inboard, __i + 1, jeast); \
	s = BOARD(inboard, __i + 1, j); \
	BOARD(outboard, __i, j) = alivep(nwe + n + we + swe + s, c); \
} while(0)

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
		}	{
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
	if (nrows != ncols || nrows < 32 || nrows > 10000 || ONETHREAD_MODE) {
		// gracefully return...
		return sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);	
	}
	
	pthread_mutex_t lock;
	pthread_mutex_init(&lock, NULL);
	pthread_cond_t cond;
	pthread_cond_init(&cond, NULL);

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
	// create threads
	pthread_create(&tid[0], NULL, top_worker, (void *)&arg[0]);	
	for (int i = 1; i < NUM_THREADS - 1; i++) {
		// create threads
		pthread_create(&tid[i], NULL, middle_worker, (void *)&arg[i]);	
	}
	// create threads
	pthread_create(&tid[NUM_THREADS - 1], NULL, bottom_worker, (void *)&arg[NUM_THREADS - 1]);	

	for (int i = 0; i < NUM_THREADS; i++) {
		// wait until all threads finished
    	pthread_join(tid[i], NULL);
  	}
	
	return inboard;
}
