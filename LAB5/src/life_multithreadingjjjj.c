/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <stdio.h>
#include <pthread.h>

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

#define BOARD( __board, i, __j )  (__board[(i) + LDA*(__j)])

// test machine is 8-core
#define NUM_THREADS 8

typedef struct tagArg {
	char *outboard;
	char *inboard;
	int nrows;
	int ncols;
	int col_start;
	int col_end;
	int LDA;
	int gens_max;
	pthread_barrier_t *barrier;
} Arg;

#define UNROLL( __i, __isouth ) do {\
				jnw = jw; jn = jc; jjn = jjc; jjjn = jjjc; jjjjn = jjjjc; jjjjne = jjjje;\
				jw = jsw; jc = js; jjc = jjs; jjjc = jjjs; jjjjc = jjjjs; jjjje = jjjjse;\
				jsw = BOARD(inboard, __isouth, jwest);\
				js = BOARD(inboard, __isouth, j);\
				jjs = BOARD(inboard, __isouth, jj);\
				jjjs = BOARD(inboard, __isouth, jjj);\
				jjjjs = BOARD(inboard, __isouth, jjjj);\
				jjjjse = BOARD(inboard, __isouth, jjjjeast);\
				BOARD(outboard, __i, j) = alivep(jnw + jn + jjn + jw + jjc + jsw + js + jjs, jc); \
				BOARD(outboard, __i, jj) = alivep(jn + jjn + jjjn + jc + jjjc + js + jjs + jjjs, jjc); \
				BOARD(outboard, __i, jjj) = alivep(jjn + jjjn + jjjjn + jjc + jjjjc + jjs + jjjs + jjjjs, jjjc); \
				BOARD(outboard, __i, jjjj) = alivep(jjjn + jjjjn + jjjjne + jjjc + jjjje + jjjs + jjjjs + jjjjse, jjjjc); \
} while(0)

void *worker(void *arg_p) {
	// read the args
	Arg *arg = (Arg *)arg_p;
	char *outboard = arg->outboard;
	char *inboard = arg->inboard;
	const int nrows = arg->nrows;
	const int ncols = arg->ncols;
	const int col_start = arg->col_start;
	const int col_end = arg->col_end;
	const int LDA = arg->LDA;
	const int gens_max = arg->gens_max;
	pthread_barrier_t *barrier = arg->barrier;
	
	char jnw, jn, jjn, jjjn, jjjjn, jjjjne;
	char jw,  jc, jjc, jjjc, jjjjc, jjjje;
	char jsw, js, jjs, jjjs, jjjjs, jjjjse;
	int i, j;
	// per-thread sequential part
	for (int curgen = 0; curgen < gens_max; curgen++) {
		// the image is column-major in memory, so is the loop		
		
		for (j = col_start; j <= col_end - 4; j+=4) {
			int jj = j + 1;
			int jjj = jj + 1;
			int jjjj = jjj + 1;

			const int jwest = mod(j-1, ncols);
			const int jjjjeast = mod(jjjj+1, ncols);
			
			jnw = BOARD(inboard, nrows - 2, jwest);
			jn = BOARD(inboard, nrows - 2, j);
			jjn = BOARD(inboard, nrows - 2, jj);		
			jjjn = BOARD(inboard, nrows - 2, jjj);	
			jjjjn = BOARD(inboard, nrows - 2, jjjj);
			jjjjne = BOARD(inboard, nrows - 2, jjjjeast);

			jw = BOARD(inboard, nrows - 1, jwest);
			jc = BOARD(inboard, nrows - 1, j);
			jjc = BOARD(inboard, nrows - 1, jj);		
			jjjc = BOARD(inboard, nrows - 1, jjj);	
			jjjjc = BOARD(inboard, nrows - 1, jjjj);
			jjjje = BOARD(inboard, nrows - 1, jjjjeast);

			jsw = BOARD(inboard, 0, jwest);
			js = BOARD(inboard, 0, j);
			jjs = BOARD(inboard, 0, jj);		
			jjjs = BOARD(inboard, 0, jjj);	
			jjjjs = BOARD(inboard, 0, jjjj);
			jjjjse = BOARD(inboard, 0, jjjjeast);

			BOARD(outboard, nrows - 1, j) = alivep(jnw + jn + jjn + jw + jjc + jsw + js + jjs, jc); 
			BOARD(outboard, nrows - 1, jj) = alivep(jn + jjn + jjjn + jc + jjjc + js + jjs + jjjs, jjc); 
			BOARD(outboard, nrows - 1, jjj) = alivep(jjn + jjjn + jjjjn + jjc + jjjjc + jjs + jjjs + jjjjs, jjjc); 
			BOARD(outboard, nrows - 1, jjjj) = alivep(jjjn + jjjjn + jjjjne + jjjc + jjjje + jjjs + jjjjs + jjjjse, jjjjc); 

			for (i = 0; i < nrows - 4; i += 4) {				
				UNROLL(i  , i+1);		
				UNROLL(i+1, i+2);
				UNROLL(i+2, i+3);	
				UNROLL(i+3, i+4);
				UNROLL(i+4, i+5);	
				UNROLL(i+5, i+6);	
				UNROLL(i+6, i+7);	
				UNROLL(i+7, i+8);	
			}
				UNROLL(i  , i+1);		
				UNROLL(i+1, i+2);
				UNROLL(i+2, i+3);	
				UNROLL(i+3, i+4);
				UNROLL(i+4, i+5);	
				UNROLL(i+5, i+6);	
				UNROLL(i+6, i+7);
		}	
		SWAP_BOARDS(outboard, inboard);
		pthread_barrier_wait(barrier);
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
	if (nrows != ncols || nrows < 32 || nrows > 10000) {
		// gracefully exit...
		fprintf(stderr, "You should provide an (N,N) image, where 32<=N<=10000\n");
		return NULL;	
	}
	
	pthread_barrier_t barrier;
	pthread_barrier_init(&barrier, NULL, NUM_THREADS);	
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
		arg[i].LDA = nrows;
		arg[i].gens_max = gens_max;
		arg[i].barrier = &barrier;
	}
	
	for (int i = 0; i < NUM_THREADS; i++) {
		// create threads
		pthread_create(&tid[i], NULL, worker, (void *)&arg[i]);	
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		// wait until all threads finished
    	pthread_join(tid[i], NULL);
  	}
	
	return inboard;
}



/* sequential implementation	
char*
_game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
  return sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);
}*/	

