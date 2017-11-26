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

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])

typedef struct tagArgs {
	char **outboard;
	char **inboard;
	int nrows;
	int ncols;
} Args;

void *nw_gol (void *argp)
{
	Args *args = (Args *) argp;
	char* outboard = *(args->outboard);
	char* inboard = *(args->inboard);
	const int nrows = args->nrows;
	const int ncols = args->ncols;

    const int LDA = nrows;
    for (int i = 0; i < nrows / 2; i++) {
		// north and west wraps around
		const int inorth = mod (i-1, nrows);
		const int isouth = i+1;
		for (int j = 0; j < ncols / 2; j++) {
			const int jwest = mod (j-1, ncols);
			const int jeast = j+1;

			const char neighbor_count = 
					BOARD (inboard, inorth, jwest) + 
					BOARD (inboard, inorth, j) + 
					BOARD (inboard, inorth, jeast) + 
					BOARD (inboard, i, jwest) +
					BOARD (inboard, i, jeast) + 
					BOARD (inboard, isouth, jwest) +
					BOARD (inboard, isouth, j) + 
					BOARD (inboard, isouth, jeast);

			BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));
		}
	}
	return NULL;
}

void *ne_gol (void *argp)
{
	Args *args = (Args *) argp;
	char* outboard = *(args->outboard);
	char* inboard = *(args->inboard);
	const int nrows = args->nrows;
	const int ncols = args->ncols;

    const int LDA = nrows;
    for (int i = 0; i < nrows / 2; i++) {
		// north and east wraps around
		const int inorth = mod (i-1, nrows);
		const int isouth = i+1;
		for (int j = ncols / 2; j < ncols; j++) {
			const int jwest = j-1;
			const int jeast = mod (j+1, ncols);

			const char neighbor_count = 
					BOARD (inboard, inorth, jwest) + 
					BOARD (inboard, inorth, j) + 
					BOARD (inboard, inorth, jeast) + 
					BOARD (inboard, i, jwest) +
					BOARD (inboard, i, jeast) + 
					BOARD (inboard, isouth, jwest) +
					BOARD (inboard, isouth, j) + 
					BOARD (inboard, isouth, jeast);

			BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));
		}
	}
	return NULL;
}

void *sw_gol (void *argp)
{
	Args *args = (Args *) argp;
	char* outboard = *(args->outboard);
	char* inboard = *(args->inboard);
	const int nrows = args->nrows;
	const int ncols = args->ncols;

    const int LDA = nrows;
    for (int i = nrows / 2; i < nrows; i++) {
		// south and west wraps around
		const int inorth = i-1;
		const int isouth = mod (i+1, nrows);
		for (int j = 0; j < ncols / 2; j++) {
			const int jwest = mod (j-1, ncols);
			const int jeast = j+1;

			const char neighbor_count = 
					BOARD (inboard, inorth, jwest) + 
					BOARD (inboard, inorth, j) + 
					BOARD (inboard, inorth, jeast) + 
					BOARD (inboard, i, jwest) +
					BOARD (inboard, i, jeast) + 
					BOARD (inboard, isouth, jwest) +
					BOARD (inboard, isouth, j) + 
					BOARD (inboard, isouth, jeast);

			BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));
		}
	}
	return NULL;
}

void *se_gol (void *argp)
{
	Args *args = (Args *) argp;
	char* outboard = *(args->outboard);
	char* inboard = *(args->inboard);
	const int nrows = args->nrows;
	const int ncols = args->ncols;

    const int LDA = nrows;
    for (int i = nrows / 2; i < nrows; i++) {
		// south and east wraps around
		const int inorth = i-1;
		const int isouth = mod (i+1, nrows);
		for (int j = ncols / 2; j < ncols; j++) {
			const int jwest = j-1;
			const int jeast = mod (j+1, ncols);

			const char neighbor_count = 
					BOARD (inboard, inorth, jwest) + 
					BOARD (inboard, inorth, j) + 
					BOARD (inboard, inorth, jeast) + 
					BOARD (inboard, i, jwest) +
					BOARD (inboard, i, jeast) + 
					BOARD (inboard, isouth, jwest) +
					BOARD (inboard, isouth, j) + 
					BOARD (inboard, isouth, jeast);

			BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));
		}
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
	Args args;
	args.nrows = nrows;
	args.ncols = ncols;
	args.outboard = &outboard;
	args.inboard = &inboard;
	
	pthread_t nw_t, ne_t, sw_t, se_t;
	for (int curgen = 0; curgen < gens_max; curgen++) {	
		pthread_create(&nw_t, NULL, nw_gol, &args);
		pthread_create(&ne_t, NULL, ne_gol, &args);
		pthread_create(&sw_t, NULL, sw_gol, &args);
		pthread_create(&se_t, NULL, se_gol, &args);
		
		pthread_join(nw_t, NULL);	
		pthread_join(ne_t, NULL);	
		pthread_join(sw_t, NULL);	
		pthread_join(se_t, NULL);	
		
		SWAP_BOARDS(outboard, inboard);
	}	
	return inboard;
}



/* sequential implementation		
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
  return sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);
}*/

