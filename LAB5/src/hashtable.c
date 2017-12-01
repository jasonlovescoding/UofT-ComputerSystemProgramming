#include <malloc.h>
#include <stdint.h>
#include <strings.h> 
#include "hashtable.h"

int hash(uint64_t key, int hashcode) {
	return key;	
}

void chain_insert(PixelItem **chain, PixelItem *item) {
	PixelItem *head = *chain;
	if (head == NULL) {
		item->prev = item;
		item->next = item;
		*chain = item;
	} else {
		item->prev = head->prev;
		item->next = head;
		item->prev->next = item;
		item->next->prev = item;
	}
}

void chain_remove(PixelItem **chain, PixelItem *item) {
	if (item->next == item) {
		*chain = NULL;
	} else {
		item->next->prev = item->prev;
		item->prev->next = item->next;
		if (*chain == item) {
			*chain = item->next;		
		}
	}
}

PixelTable *table_new(int capacity) {
	PixelTable *table = (PixelTable *)malloc(sizeof(PixelTable));
	table->array = (PixelItem **)malloc(sizeof(PixelItem*) * capacity);
	bzero(table->array, sizeof(PixelItem*) * (table->capacity));
	table->dummy = (PixelItem *)malloc(sizeof(PixelItem) * capacity);
	table->capacity = capacity;
	table->chain = NULL;
	return table;
}

void table_clear(PixelTable *table) {
	table->chain = NULL;
}

bool table_insert(PixelTable *table, uint64_t key, bool from_neighbor, int curgen) {
	PixelItem **array = table->array;
	int capacity = table->capacity;
	int hash_index = hash(key, capacity);
	if (array[hash_index] != NULL && array[hash_index]->curgen == curgen) { 
		int neighbor_count = array[hash_index]->neighbor_count;
		if (neighbor_count > 3) return false;

		if (from_neighbor) {
			array[hash_index]->neighbor_count = neighbor_count + 1;
			if (neighbor_count >= 3) {
				chain_remove(&(table->chain), array[hash_index]);		
			}
		} else {
			array[hash_index]->isalive = true;			
		}
		return false;
	}
	PixelItem *item = &(table->dummy[hash_index]);
	item->key = key;
	if (from_neighbor) {
		item->isalive = false;
		item->neighbor_count = 1;
	} else {
		item->isalive = true;
		item->neighbor_count = 0;
	}
	item->curgen = curgen;
	array[hash_index] = item;
   	chain_insert(&(table->chain), item);
	return true;	
}

PixelItem *table_search(PixelTable *table, uint64_t key, int curgen) {
	PixelItem **array = table->array;
	int capacity = table->capacity;
	int hash_index = hash(key, capacity);
	while (array[hash_index] != NULL) {
		if (array[hash_index]->key == key) {
			if (array[hash_index]->curgen == curgen) {
				return array[hash_index];
			} else {
				return NULL;			
			}
		}
		hash_index++;
		hash_index %= capacity;
	}
	return NULL;
}


bool table_delete(PixelTable *table, uint64_t key) {
	PixelItem **array = table->array;
	int capacity = table->capacity;
	int hash_index = hash(key, capacity);
	
	while (array[hash_index] != NULL) {
		if (array[hash_index]->key == key) {
			PixelItem *item = array[hash_index];
			array[hash_index] = NULL;
			chain_remove(&(table->chain), item);
			return true;	
		}
		hash_index++;
		hash_index %= capacity;
	}
	return false;	
} 

void table_free(PixelTable *table) {
	free(table->array);
	free(table->dummy);
	free(table);
}
