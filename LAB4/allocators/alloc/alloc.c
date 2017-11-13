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
     "",
     /* Email address of second team member */
     "",
     /* Student Number of second team member */
     ""
};

pthread_mutex_t malloc_lock = PTHREAD_MUTEX_INITIALIZER;

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
#define NUM_SEG_LISTS 15

/* Minimum size of a block */
#define MIN_LOG2_BLOCK_SIZE 5
#define MIN_BLOCK_SIZE (1<<MIN_LOG2_BLOCK_SIZE)

/* bit masking the last bit out of a char pointer */
#define FIND_LAST_BITH(bp) (GET_ALLOC(HDRP(bp)))
#define FIND_LAST_BITF(bp) (GET_ALLOC(FTRP(bp)))

/* To achieve constant-time list_remove, we use doubly-linked circular list. */
/* To achieve constant-time list_insert into tail, we make the list circular */
typedef struct tagFreeBlock {
    struct tagFreeBlock *prev;
    // the last block should poing to the first block
    struct tagFreeBlock *next;
} FreeBlock;

/* The segregated lists */
FreeBlock* seg_lists[NUM_SEG_LISTS];

/**********************************************************
 * list_index
 * @param: int num_words (assert >= MIN_LOG2_BLOCK_SIZE)
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
 * @param: FreeBlock* bp
 * @effect: insert block *bp into corresponding list
 **********************************************************/
void list_insert(FreeBlock* bp) {
    int index = list_index(GET_SIZE(HDRP(bp)));

    if (seg_lists[index] == NULL) {
        // case 1: list is empty. Insert block pointing to itself
        bp->next = bp;
        bp->prev = bp;
        seg_lists[index] = bp;
    } else {
        // case 2: list not empty. Insert block to list tail
        bp->next = seg_lists[index];
        bp->prev = seg_lists[index]->prev;
        bp->prev->next = bp;
        bp->next->prev = bp;
    }
}

/**********************************************************
 * list_remove
 * @param: FreeBlock* bp
 * @effect: remove block *bp from corresponding list
 **********************************************************/
void list_remove(FreeBlock* bp) {
    int index = list_index(GET_SIZE(HDRP(bp)));

    if (bp->next == bp) {
        // case 1: bp is the only block in list, make list empty
        seg_lists[index] = NULL;
    } else {
        // case 2: remove bp and relink prev/next block
        bp->next->prev = bp->prev;
        bp->prev->next = bp->next;
        if (seg_lists[index] == bp) {
            // bp is head of list
            seg_lists[index] = bp->next;
        }
    }
}

void* heap_listp = NULL;
/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void)
{
	pthread_mutex_lock(&malloc_lock);
    for (int i = 0; i < NUM_SEG_LISTS; i++) {
        // initialize all lists as empty
        seg_lists[i] = NULL;
    }
	if (dseg_lo == NULL && dseg_hi == NULL) {
		pthread_mutex_unlock(&malloc_lock);
		if (mem_init()) return -1;
	}
	if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *) - 1)
        return -1;
	PUT(heap_listp, 0); // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1)); // epilogue header
    heap_listp += DSIZE;
	pthread_mutex_unlock(&malloc_lock);
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
void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) { /* Case 1 */
        return bp;
    } else if (prev_alloc && !next_alloc) { /* Case 2, with next block */
        list_remove((FreeBlock *) NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        return (bp);
    } else if (!prev_alloc && next_alloc) { /* Case 3, with prev block */
        list_remove((FreeBlock *) PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        return (PREV_BLKP(bp));
    } else { /* Case 4, with next & prev block */
        list_remove((FreeBlock *) PREV_BLKP(bp));
        list_remove((FreeBlock *) NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        return (PREV_BLKP(bp));
    }
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((bp = mem_sbrk(size)) == (void *) - 1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0)); // free block header
    PUT(FTRP(bp), PACK(size, 0)); // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // new epilogue header

    return bp;
}

/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize) {
    for (int index = list_index(asize); index < NUM_SEG_LISTS; index++) {
        FreeBlock *bp = seg_lists[index];
        if (bp == NULL) continue;
        do {
            int block_size = GET_SIZE(HDRP(bp));
            if (block_size >= asize) {
                // found a block with enough size
                int remainder_size = block_size - asize;
                // first fit
                list_remove(bp);
                if (remainder_size >= MIN_BLOCK_SIZE) {
                    // the remainder is big enough as a new block
                    void *newbp = (void *) bp + asize;
                    PUT(HDRP(newbp), PACK(remainder_size, 0));
                    PUT(FTRP(newbp), PACK(remainder_size, 0));
                    list_insert(newbp);

                    PUT(HDRP(bp), PACK(asize, 0));
                    PUT(FTRP(bp), PACK(asize, 0));
                }
                return (void *) bp;
            }
            bp = bp->next;
        } while (bp != seg_lists[index]);
    }
    return NULL;
}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize) {
    /* Get the current block size */
    size_t bsize = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(bsize, 1));
    PUT(FTRP(bp), PACK(bsize, 1));
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp) {
	pthread_mutex_lock(&malloc_lock);
    if (bp == NULL) {
		pthread_mutex_unlock(&malloc_lock);
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    list_insert((FreeBlock *) coalesce(bp));
	pthread_mutex_unlock(&malloc_lock);
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
	pthread_mutex_lock(&malloc_lock);
    size_t asize; /* adjusted block size */
    char * bp;
	
    /* Ignore spurious requests */
    if (size == 0) {
		pthread_mutex_unlock(&malloc_lock);
        return NULL;
    }

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE) {
        asize = 2 * DSIZE;
    } else {
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
		pthread_mutex_unlock(&malloc_lock);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    if ((bp = extend_heap(asize / WSIZE)) == NULL) { 
		pthread_mutex_unlock(&malloc_lock);		
		return NULL;
	}
    place(bp, asize);
	pthread_mutex_unlock(&malloc_lock);
    return bp;
}

