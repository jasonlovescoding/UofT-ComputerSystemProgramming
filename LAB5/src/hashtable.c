#include <malloc.h>
#include <stdint.h>
#include <strings.h> 
#include "hashtable.h"

int hash(uint64_t key, int hashcode) {
	return key % hashcode;	
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
	free(item);
}

void chain_free(PixelItem **chain) {
	PixelItem *item = *chain;
	if (item == NULL) return;
	PixelItem *cur;
	do {
		cur = item;
		item = item->next;
		free(cur);
	} while (item != *chain);
	*chain = NULL;
}

PixelTable *table_new(int capacity) {
	PixelTable *table = (PixelTable *)malloc(sizeof(PixelTable));
	table->array = (PixelItem **)malloc(sizeof(PixelItem*) * capacity);
	bzero(table->array, sizeof(PixelItem*) * (table->capacity));
	table->capacity = capacity;
	table->chain = NULL;
	table->size = 0;
	return table;
}

void table_clear(PixelTable *table) {
	PixelItem **array = table->array;
	int capacity = table->capacity;

	PixelItem *item = table->chain;
	do {
		int hash_index = hash(item->key, capacity);	
		while (array[hash_index] != NULL) {
			if (array[hash_index]->key == item->key) {
				array[hash_index] = NULL;
				break;	
			}
			hash_index++;
			hash_index %= capacity;
		}
		item = item->next;
	} while (item != table->chain);

	chain_free(&(table->chain));
	table->size = 0;
}

bool table_insert(PixelTable *table, uint64_t key, Pixel pixel) {
	PixelItem **array = table->array;
	int capacity = table->capacity;
	int hash_index = hash(key, capacity);
	while (array[hash_index] != NULL) {
		if (array[hash_index]->key == key) {
			return false;
		}
		hash_index++;
		hash_index %= capacity;
	}
	PixelItem *item = (PixelItem *)malloc(sizeof(PixelItem));
	item->key = key;
	item->pixel = pixel;
	table->size++;
	array[hash_index] = item;
   	chain_insert(&(table->chain), item);
	return true;	
}

PixelItem *table_search(PixelTable *table, uint64_t key) {
	PixelItem **array = table->array;
	int capacity = table->capacity;
	int hash_index = hash(key, capacity);
	while (array[hash_index] != NULL) {
		if (array[hash_index]->key == key) {
			return array[hash_index];
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
			table->size--;
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
	chain_free(&(table->chain));
	free(table->array);
	free(table);
}
