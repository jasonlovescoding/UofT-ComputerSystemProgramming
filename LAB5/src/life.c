/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include "hashtable.h"

/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/
/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_TABLES( t1, t2 )  do { \
  PixelTable* temp = t1; \
  t1 = t2; \
  t2 = temp; \
} while(0)

#define INDEX( __i, __j )  ((__i) + LDA*(__j))

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])

void revives_community(char *inboard, PixelTable *revives, int i, int j, int nrows, int ncols) {
	const int LDA = nrows;
	const int inorth = mod (i-1, nrows);
	const int isouth = mod (i+1, nrows);		
	const int jwest = mod (j-1, ncols);
	const int jeast = mod (j+1, ncols);
	
	Pixel nw;
	nw.row = inorth;
	nw.col = jwest;
	table_insert(revives, (uint64_t)INDEX(inorth, jwest), nw);

	Pixel n;
	n.row = inorth;
	n.col = j;
	table_insert(revives, (uint64_t)INDEX(inorth, j), n);	

	Pixel ne;
	ne.row = inorth;
	ne.col = jeast;
	table_insert(revives, (uint64_t)INDEX(inorth, jeast), ne);

	Pixel w;
	w.row = i;
	w.col = jwest;
	table_insert(revives, (uint64_t)INDEX(i, jwest), w);

	Pixel centre;
	centre.row = i;
	centre.col = j;
	table_insert(revives, (uint64_t)INDEX(i, j), centre);

	Pixel e;
	e.row = i;
	e.col = jeast;
	table_insert(revives, (uint64_t)INDEX(i, jeast), e);

	Pixel sw;
	sw.row = isouth;
	sw.col = jwest;
	table_insert(revives, (uint64_t)INDEX(isouth, jwest), sw);

	Pixel s;
	s.row = isouth;
	s.col = j;
	table_insert(revives, (uint64_t)INDEX(isouth, j), s);

	Pixel se;	
	se.row = isouth;
	se.col = jeast;
	table_insert(revives, (uint64_t)INDEX(isouth, jeast), se);
}

void detect_pixels(char *inboard, int nrows, int ncols, PixelTable **alives, PixelTable **revives) {
	*alives = table_new(nrows * ncols);
	*revives = table_new(nrows * ncols);
	const int LDA = nrows;

	for (int i = 0; i < nrows; i++) {
		for (int j = 0; j < ncols; j++) {
			char alive = BOARD (inboard, i, j);
			if (alive) {
				Pixel centre;
				centre.row = i;
				centre.col = j;
				table_insert(*alives, (uint64_t)INDEX(i, j), centre);
				revives_community(inboard, *revives, i, j, nrows, ncols);		
			}
		}
	}
}

void dump_pixels(char *outboard, PixelTable *alives, int nrows, int ncols) {
	bzero(outboard, nrows * ncols);
	PixelItem *item = alives->chain;
	const int LDA = nrows;
	if (item != NULL) {
		do {
			Pixel pixel = item->pixel;
			int row = pixel.row;
			int col = pixel.col;
			BOARD(outboard, row, col) = 1;
			item = item->next;		
		} while (item != alives->chain);
	}
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
	PixelTable *inalives, *inrevives;
	detect_pixels(inboard, nrows, ncols, &inalives, &inrevives);	
	PixelTable *outalives, *outrevives;
	outalives = table_new(nrows * ncols);
	outrevives = table_new(nrows * ncols);

	const int LDA = nrows;
	for (int curgen = 0; curgen < gens_max; curgen++) {
		PixelItem *item = inrevives->chain; 		
		if (item != NULL) {
			do {
				Pixel pixel = item->pixel;
				int i = pixel.row;
				int j = pixel.col;
				
				const int inorth = mod (i-1, nrows);
				const int isouth = mod (i+1, nrows);		
				const int jwest = mod (j-1, ncols);
				const int jeast = mod (j+1, ncols);

				char neighbor_count = 0;				
				if (table_search(inalives, (uint64_t)INDEX(inorth, jwest))) neighbor_count++;
				if (table_search(inalives, (uint64_t)INDEX(inorth, j))) neighbor_count++;
				if (table_search(inalives, (uint64_t)INDEX(inorth, jeast))) neighbor_count++;	
				if (table_search(inalives, (uint64_t)INDEX(i, jwest))) neighbor_count++;
				if (table_search(inalives, (uint64_t)INDEX(i, jeast))) neighbor_count++;
				if (table_search(inalives, (uint64_t)INDEX(isouth, jwest))) neighbor_count++;
				if (table_search(inalives, (uint64_t)INDEX(isouth, j))) neighbor_count++;	
				if (table_search(inalives, (uint64_t)INDEX(isouth, jeast))) neighbor_count++;
				
				char alive = 0;
				if (table_search(inalives, (uint64_t)INDEX(i, j))) alive = 1;
				alive = alivep (neighbor_count, alive);		

				if (alive) {
					Pixel centre;
					centre.row = i;
					centre.col = j;
					table_insert(outalives, (uint64_t)INDEX(i, j), centre);
					revives_community(inboard, outrevives, i, j, nrows, ncols);					
				}
				item = item->next;
			} while (item != inrevives->chain);
		}
	
		table_clear(inalives);
		table_clear(inrevives);	
		SWAP_TABLES(outalives, inalives);
		SWAP_TABLES(outrevives, inrevives);	
	}

	dump_pixels(outboard, inalives, nrows, ncols);
	table_free(inalives);
	table_free(inrevives);
	table_free(outalives);
	table_free(outrevives);
	return outboard;
}


/* sequential implementation	*/
char*
m_game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
  return sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);
}

