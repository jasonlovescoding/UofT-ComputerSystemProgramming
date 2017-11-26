#ifndef _hashtable_h
#define _hashtable_h

#include <stdbool.h>
#include <stdint.h>

typedef struct tagPixel {
	int row;
	int col;
} Pixel;

typedef struct tagPixelItem {
	uint64_t key;
	struct tagPixel pixel;
	struct tagPixelItem *prev;
	struct tagPixelItem *next;
} PixelItem;

typedef struct tagPixelTable {
	struct tagPixelItem **array;
	struct tagPixelItem *chain;
	int size;
	int capacity;
} PixelTable;

PixelTable *table_new(int capacity);

void table_clear(PixelTable *table);

PixelItem *table_search(PixelTable *table, uint64_t key);

bool table_insert(PixelTable *table, uint64_t key, Pixel pixel);

bool table_delete(PixelTable *table, uint64_t key);

void table_free(PixelTable *table);

#endif
