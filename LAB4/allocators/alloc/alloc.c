#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#include "memlib.h"
#include "malloc.h"

name_t myname = {
     /* team name to be displayed on webpage */
     "C99",
     /* Full name of first team member */
     "ZHANG QIANHAO",
     /* Email address of first team member */
     "qianhao.zhang@mail.utoronto.ca",
     /* Student Number of first team member */
     "1004654377"
     /* Full name of second team member */
     "CHEN JINGFENG",
     /* Email address of second team member */
     "jingfeng.chen@mail.utoronto.ca",
     /* Student Number of second team member */
     "1000411262"
};

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
 *************************************************************************/
#define WSIZE       sizeof(void *)            /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(u_int32_t *)(p))
#define PUT(p,val)      (*(u_int32_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Number of segregated lists */
#define NUM_SEG_LISTS 10

/* Minimum size of a block */
#define MIN_LOG2_BLOCK_SIZE 5
#define MIN_BLOCK_SIZE (1 << MIN_LOG2_BLOCK_SIZE)

/* Minimum size of a malloc without rounding up */
#define MIN_MALLOC_BLOCK_SIZE (MIN_BLOCK_SIZE << 4)

// -------- A quick implementation of hash table for mapping tid into integer id
#define NUM_THREADS 9
int num_thread = 0;
typedef struct TagThreadItem {
   pthread_t tid;   
   int id;
} ThreadItem;

ThreadItem* hashArray[NUM_THREADS]; 
ThreadItem items[NUM_THREADS];
pthread_mutex_t hashtable_lock;

int hashcode(pthread_t tid) {
   return ((u_int64_t) tid) % NUM_THREADS;
}

ThreadItem *hashtable_search(pthread_t tid) {
   //get the hash 
   int hashIndex = hashcode(tid);  
	
   //move in array until an empty 
   while(hashArray[hashIndex] != NULL) {
	
      if(pthread_equal(hashArray[hashIndex]->tid, tid)) {
         return hashArray[hashIndex]; 
	  }
			
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= NUM_THREADS;
   }        
	
   return NULL;        
}

ThreadItem *hashtable_insert(pthread_t tid) {
   ThreadItem *item = &items[num_thread];
   item->tid = tid;  
   item->id = num_thread;
   num_thread++;

   // get the hash 
   int hashIndex = hashcode(tid);

   // move in array until an empty cell
   while(hashArray[hashIndex] != NULL) {
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= NUM_THREADS;
   }
	
   hashArray[hashIndex] = item;
   return item;
}

int thread_id() {
	pthread_t tid = pthread_self();
	ThreadItem *item = hashtable_search(tid);
	if (item != NULL) {
		return item->id; 	
	} else {
		pthread_mutex_lock(&hashtable_lock);
		item = hashtable_insert(tid);	
		pthread_mutex_unlock(&hashtable_lock);
		return item->id;
	}
}
// -------- ref: https://www.tutorialspoint.com/data_structures_algorithms/hash_table_program_in_c.htm

// -------- Segregated lists still stores blocks
typedef struct tagFreeBlock {
    struct tagFreeBlock *prev;
    struct tagFreeBlock *next;
} FreeBlock;

typedef struct tagSegList {
	struct tagFreeBlock *list[NUM_SEG_LISTS];
	u_int64_t lo;
	u_int64_t hi;
} SegList;

// 512 KBytes per thread heap
#define THREAD_HEAPSIZE (262144)
/* The segregated lists of threads */
SegList seg_lists[NUM_THREADS];

// --------

// -------- big objects are allocated with unit pages
// the biggest object supported is 16-page large
#define NUM_PAGE_LISTS 8
// page size is 4KB
#define PAGE_SIZE 0x1000
// the max block is 32KB, 8-page large
#define MAX_BLOCK_SIZE 0x8000
// so the minimal page-aligned block is 9-page large
#define MIN_CNT_PAGE_SIZE 9 
#define MIN_PAGE_SIZE 0x9000

FreeBlock* page_lists[NUM_PAGE_LISTS];
pthread_mutex_t pages_lock[NUM_PAGE_LISTS];
// --------

pthread_mutex_t mem_lock;
void* heap_listp = NULL;

/**********************************************************
 * page_index
 * @param: adjusted size of object
 * @return: index of page list with node of corresponding adjusted size
 **********************************************************/
int page_index(int asize) {
	int index = 0;
	while ((asize -= PAGE_SIZE) > 0) index++;
	return MAX(0, MIN((index - MIN_CNT_PAGE_SIZE + 1), (NUM_PAGE_LISTS - 1)));
}

/**********************************************************
 * page_insert
 * @param: index of list, pointer of page-aligned block
 * @effect: insert block *bp into corresponding list
 **********************************************************/
void page_insert(int index, FreeBlock* bp) {
    if (page_lists[index] == NULL) {
        // case 1: list is empty. Insert page pointing to itself
        bp->next = bp;
        bp->prev = bp;
        page_lists[index] = bp;
    } else {
        // case 2: list not empty. Insert page to list tail
        bp->next = page_lists[index];
        bp->prev = page_lists[index]->prev;
        bp->prev->next = bp;
        bp->next->prev = bp;
    }
}

/**********************************************************
 * page_remove
 * @param: index of list, pointer of page-aligned block
 * @effect: remove block *bp from corresponding list
 **********************************************************/
void page_remove(int index, FreeBlock* bp) {
    if (bp->next == bp) {
        // case 1: bp is the only block in list, make list empty
        page_lists[index] = NULL;
    } else {	
        // case 2: remove bp and relink prev/next page-aligned block
        bp->next->prev = bp->prev;
        bp->prev->next = bp->next;
        if (page_lists[index] == bp) {
            // bp is head of list
            page_lists[index] = bp->next;
        }
    }
}

/**********************************************************
 * list_index
 * @param: adjusted size of object
 * @return: index of segregated list
                        with node of corresponding adjusted size
 **********************************************************/
int list_index(int asize) {
    int index = 0;
    while (asize >>= 1) index++;
    return MAX(0, MIN((index - MIN_LOG2_BLOCK_SIZE + 1), (NUM_SEG_LISTS - 1)));
}

/**********************************************************
 * list_insert
 * @param: id of thread, index of list, pointer of block
 * @effect: insert block *bp into corresponding list
 **********************************************************/
void list_insert(int id, int index, FreeBlock* bp) {
	FreeBlock **seg_list = seg_lists[id].list;
    if (seg_list[index] == NULL) {
        // case 1: list is empty. Insert block pointing to itself
        bp->next = bp;
        bp->prev = bp;
        seg_list[index] = bp;
    } else {
        // case 2: list not empty. Insert block to list tail
        bp->next = seg_list[index];
        bp->prev = seg_list[index]->prev;
        bp->prev->next = bp;
        bp->next->prev = bp;
    }
}

/**********************************************************
 * list_remove
 * @param: id of thread, index of list, pointer of block
 * @effect: remove block *bp from corresponding list
 **********************************************************/
void list_remove(int id, int index, FreeBlock* bp) {
	FreeBlock **seg_list = seg_lists[id].list;
    if (bp->next == bp) {
        // case 1: bp is the only block in list, make list empty
        seg_list[index] = NULL;
    } else {	
        // case 2: remove bp and relink prev/next block
        bp->next->prev = bp->prev;
        bp->prev->next = bp->next;
        if (seg_list[index] == bp) {
            // bp is head of list
            seg_list[index] = bp->next;
        }
    }
}

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void)
{
	if (dseg_lo == NULL && dseg_hi == NULL) {
		if (mem_init()) return -1;
	}
	pthread_mutex_init(&hashtable_lock, NULL);
	pthread_mutex_init(&mem_lock, NULL);
	for (int index = 0; index < NUM_SEG_LISTS; index++) {
			// initialize all locks
        	pthread_mutex_init(&pages_lock[index], NULL);
		}

	for (int id = 0; id < NUM_THREADS; id++) {
		for (int index = 0; index < NUM_SEG_LISTS; index++) {
			// initialize all lists as empty
        	seg_lists[id].list[index] = NULL;
		}
    }

	for (int id = 0; id < NUM_THREADS; id++) {
		for (int index = 0; index < NUM_SEG_LISTS; index++) {
			// initialize the thread heap for a new thread		
			if ((heap_listp = mem_sbrk(THREAD_HEAPSIZE)) == (void *) - 1) {
			    return -1;
			}
			PUT(heap_listp, 0); // alignment padding
			PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // prologue header
			PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // prologue footer
			void *bp = heap_listp + (4 * WSIZE);
			PUT(HDRP(bp), PACK(THREAD_HEAPSIZE - (8 * WSIZE), 0)); // initial block header
			PUT(FTRP(bp), PACK(THREAD_HEAPSIZE - (8 * WSIZE), 0)); // initial block header
			PUT(FTRP(bp) + WSIZE, PACK(0, 1)); // epilogue header
		
			list_insert(id, NUM_SEG_LISTS - 1, bp); // insert initial block for the heap
			
			seg_lists[id].lo = (u_int64_t) bp;
			seg_lists[id].hi = ((u_int64_t) bp) + THREAD_HEAPSIZE - (8 * WSIZE);
		}
	}

	for (int index = 0; index < NUM_PAGE_LISTS; index++) {
		// initialize all page lists as empty
		page_lists[index] = NULL;	
	}

	// initialize the global heap
	if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *) - 1) {
    	    return -1;
	}
	PUT(heap_listp, 0); // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1)); // epilogue header
	return 0;
}

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/
void *coalesce(int id, void *bp) {
	size_t size = GET_SIZE(HDRP(bp));

	size_t prev_size = GET_SIZE(HDRP(PREV_BLKP(bp)));
	int prev_index = list_index(prev_size);

	size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));	
	if (!prev_alloc) { 		
		list_remove(id, prev_index, (FreeBlock *) PREV_BLKP(bp));
		size += prev_size;
		PUT(FTRP(bp), PACK(size, 1));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 1));
		bp = (PREV_BLKP(bp));	
	} 	

	size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(bp)));
	int next_index = list_index(next_size);
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	if (!next_alloc) { 
        list_remove(id, next_index, (FreeBlock *) NEXT_BLKP(bp));
        size += next_size;
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
	}

	return bp;
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words) {
	pthread_mutex_lock(&mem_lock);
    char *bp;
    size_t size;
    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((bp = mem_sbrk(size)) == (void *) - 1) {
		pthread_mutex_unlock(&mem_lock);        
		return NULL;	
	}
	
    /* Initialize block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 1)); // alloc block header
    PUT(FTRP(bp), PACK(size, 1)); // alloc block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // new epilogue header
	pthread_mutex_unlock(&mem_lock);
    return bp;
}

/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize) {
	int id = thread_id();
	FreeBlock **seg_list = seg_lists[id].list;
    for (int index = list_index(asize); index < NUM_SEG_LISTS; index++) {
        FreeBlock *bp = seg_list[index];
        if (bp == NULL) { 		
			continue;
		}
			
        do {
            int block_size = GET_SIZE(HDRP(bp));
            if (block_size >= asize) {
                // found a block with enough size
                int remainder_size = block_size - asize;
                // first fit
                list_remove(id, index, bp);
				PUT(HDRP(bp), PACK(block_size, 1));
                PUT(FTRP(bp), PACK(block_size, 1));

                if (remainder_size >= MIN_BLOCK_SIZE) {			
                    // the remainder is big enough as a new block.		
					int remainder_index = list_index(remainder_size);

                    void *newbp = (void *) bp + asize;					
                    PUT(HDRP(newbp), PACK(remainder_size, 0));
                    PUT(FTRP(newbp), PACK(remainder_size, 0));
                    list_insert(id, remainder_index, newbp);

                    PUT(HDRP(bp), PACK(asize, 1));
                    PUT(FTRP(bp), PACK(asize, 1));		
                }
                return (void *) bp;
            }
            bp = bp->next;
        } while (bp != seg_list[index]);
    }
    return NULL;
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp) {
    if (bp == NULL) {
        return;
    }
	
	size_t asize = GET_SIZE(HDRP(bp));
	if (asize <= MAX_BLOCK_SIZE) {
		// if a small object, same as lab3
		int id = thread_id();
		if ((u_int64_t)bp < seg_lists[id].hi && (u_int64_t)bp >= seg_lists[id].lo) {
			// if within thread cache, try coalescing		
			bp = coalesce(id, bp);
			asize = GET_SIZE(HDRP(bp));
		}
		int index = list_index(asize);
		PUT(HDRP(bp), PACK(asize, 0));
		PUT(FTRP(bp), PACK(asize, 0));
		list_insert(id, index, bp);
	} else {
		// else, insert back into page lists
		int index = page_index(asize);
		pthread_mutex_lock(&pages_lock[index]);
		PUT(HDRP(bp), PACK(asize, 0));
		PUT(FTRP(bp), PACK(asize, 0));
		page_insert(index, bp);
		pthread_mutex_unlock(&pages_lock[index]);
	}
}

/**********************************************************
 * find_fit
 * Traverse the heap searching for a page-aligned block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void *find_page(int asize) {
	int index = page_index(asize);
	FreeBlock *bp = page_lists[index];
	if (bp == NULL) {
		return NULL;
	} else {
		// there is no fragment handling for big objects
		// block size is alway page-aligned
		pthread_mutex_lock(&pages_lock[index]);
		PUT(HDRP(bp), PACK(asize, 0));
		PUT(FTRP(bp), PACK(asize, 0));
		page_remove(index, bp);
		pthread_mutex_unlock(&pages_lock[index]);
		return bp;
	}
}

/**********************************************************
 * extend_page
 * Extend the heap by bytes asize, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_page(int asize) {
	pthread_mutex_lock(&mem_lock);
    char *bp;

    /* Allocate an even number of words to maintain alignments */
    if ((bp = mem_sbrk(asize)) == (void *) - 1) {
		pthread_mutex_unlock(&mem_lock);        
		return NULL;	
	}
	
    /* Initialize block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(asize, 1)); // alloc block header
    PUT(FTRP(bp), PACK(asize, 1)); // alloc block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // new epilogue header
	pthread_mutex_unlock(&mem_lock);
    return bp;
}


/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size) {
    size_t asize; /* adjusted block size */
    char * bp;
    /* Ignore spurious requests */
    if (size == 0) {
        return NULL;
    }
	
	if (size < (MIN_MALLOC_BLOCK_SIZE)) {
        int rsize = 1;
        while (rsize < size) {
            rsize <<= 1;
        }
        size = rsize;
    }

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE) {
        asize = (DSIZE << 1);
    } else {
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }

	if (asize <= MAX_BLOCK_SIZE) {
		// if a small object, same as lab3
		/* Search the free list for a fit */
		if ((bp = find_fit(asize)) != NULL) {
		    return bp;
		}
		/* No fit found. Get more memory and place the block */
		if ((bp = extend_heap(asize / WSIZE)) == NULL) { 	
			return NULL;
		}
		return bp;
	} else {
		// else, round the asize up to be page-aligned 
		asize = ((asize + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
		/* Search the free list for a fit */
		if ((bp = find_page(asize)) != NULL) {
		    return bp;
		}
		/* No fit found. Get more memory and place the page-aligned block */
		if ((bp = extend_page(asize)) == NULL) { 	
			return NULL;
		}
		return bp;
	}
}
