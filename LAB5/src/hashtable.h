#ifndef _hashtable_h
#define _hashtable_h

#include <stdbool.h>
#include <stdint.h>

typedef struct tagPixelItem {
	uint64_t key;
	int neighbor_count;
	int curgen;
	bool isalive;
	struct tagPixelItem *prev;
	struct tagPixelItem *next;
} PixelItem;

typedef struct tagPixelTable {
	struct tagPixelItem **array;
	struct tagPixelItem *chain;
	struct tagPixelItem *dummy;
	int capacity;
} PixelTable;

PixelTable *table_new(int capacity);

void table_clear(PixelTable *table);

PixelItem *table_search(PixelTable *table, uint64_t key, int curgen);

bool table_insert(PixelTable *table, uint64_t key, bool from_neighbor, int curgen);

bool table_delete(PixelTable *table, uint64_t key);

void table_free(PixelTable *table);

#endif
