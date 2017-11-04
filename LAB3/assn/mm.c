/* Block management: segregated lists to achieve utilization close to the level of tree management, which is not applicable given the limit of global variables. 
 * List structure: doubly-linked list to achieve constant-time node removal.
 * Block structure: 
 *       Allocated block: header | payload | (padding) | footer 
 *       Free block: header | prev pointer | next pointer | payload | footer
 * Search algorithm: first-fit to maximize throughput. utilization is improved via rounding skills.
 * Rounding skill: 1. External fragments larger than MIN_BLOCK_SIZE are re-inserted as new blocks to improve utilization without large cost on throughput.
 *                 2. malloc's smaller than MIN_MALLOC_BLOCK_SIZE are rounded up to nearest power-2 number to maximize the chance of reuse with little cost and thus increase utilization.
 * Side-note: given that external fragments are often generated in realloc, with the rounding skill #1 there are usually a lot of smaller blocks re-inserted.
 *            To improve the performance of realloc, we choose to insert new blocks into the tail of segregated lists to speed up search.
 *            We make the lists circular to achieve constant-time tail insertion.
 * List overview: there are altogether NUM_SEG_LISTS segregated lists. 
 * The first list contains blocks of size [(1<<MIN_LOG2_BLOCK_SIZE), (1<<MIN_LOG2_BLOCK_SIZE + 1))
 * Each list later contains blocks of lower and upper bound doubled from the previous list.
 * Consistency check:
 * int check_fb_property(); contains the following:
 * 1. check the last bit of the header/footer of a free block is 0;
 * 2. check if all free blocks are in the lists;
 * 3. check if any of the free blocks are overlapped with each other in the heap;
 * 
 * int check_coalesce(); contains the following:
 * 1. check if all free blocks are coalesced properly;
 * 
 * int check_realloc_consistency(void *old_p, void *ptr); contains the following:
 * 1. check after reallocation, if the new data is consistent with the old data;
 * where the old data is preserved by void *store_data(void *old_p);
 * 
 * int mm_check(); is composed by check_coalesce() and check_fb_property().
 * But check_realloc_consistency should only be provoked in mm_realloc, so we 
 * decide not to put it in mm_check.
 * 
 * Caveat:
 * All the three check funcions will significantly slow down the procedure.
 * YOU SHOULD ONLY USE THEM ON SMALL TRACE FILES.
 * They are included in the program but commented out.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "C99",
    /* First member's full name */
    "ZHANG QIANHAO",
    /* First member's email address */
    "qianhao.zhang@mail.utoronto.ca",
    /* Second member's full name (leave blank if none) */
    "Chen Jingfeng",
    /* Second member's email address (leave blank if none) */
    "jingfeng.chen@mail.utoronto.ca"
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
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p,val)      (*(uintptr_t *)(p) = (val))

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
#define MIN_BLOCK_SIZE (1<<MIN_LOG2_BLOCK_SIZE)

/* Minimum size of a malloc without rounding up */
#define MIN_MALLOC_BLOCK_SIZE (MIN_BLOCK_SIZE << 4)

/* bit masking the last bit out of a char pointer */
#define FIND_LAST_BITH(bp) (GET_ALLOC(HDRP(bp)))
#define FIND_LAST_BITF(bp) (GET_ALLOC(FTRP(bp)))

void* heap_listp = NULL;

/* To achieve constant-time list_remove, we use doubly-linked circular list. */

/* To achieve constant-time list_insert into tail, we make the list circular */
typedef struct tagFreeBlock {
    struct tagFreeBlock *prev;
    // the last block should poing to the first block
    struct tagFreeBlock *next;
} FreeBlock;

/* The segregated lists */
FreeBlock* seg_lists[NUM_SEG_LISTS];

void *store_data(void *old_p);
int check_realloc_consistency(void *old_p, void *ptr);
int check_fb_property();
int check_coalesce();
int mm_check();

/**********************************************************
 * list_index
 * @param: int num_words (assert >= MIN_LOG2_BLOCK_SIZE)
 * @return: index of segregated list
                        with node of corresponding adjusted size
 **********************************************************/
int list_index(int asize) {
    int index = 0;
    while (asize >>= 1) index++;
    return MIN((index - MIN_LOG2_BLOCK_SIZE + 1), (NUM_SEG_LISTS - 1));
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
        // case 2: list not empty. Insert block to list head
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

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void) {
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *) - 1)
        return -1;
    PUT(heap_listp, 0); // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1)); // epilogue header
    heap_listp += DSIZE;

    for (int i = 0; i < NUM_SEG_LISTS; i++) {
        // initialize all lists as empty
        seg_lists[i] = NULL;
    }
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
    if (bp == NULL) {
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    list_insert((FreeBlock *) coalesce(bp));
    //mm_check();
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
    // To maximize utilization with little cost on throughput
    // we can round small malloc task size UP to nearest 2-pow exp
    if (size < (MIN_MALLOC_BLOCK_SIZE)) {
        int rsize = 1;
        while (rsize < size) {
            rsize <<= 1;
        }
        size = rsize;
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
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    if ((bp = extend_heap(asize / WSIZE)) == NULL) return NULL;
    place(bp, asize);
    //mm_check();
    return bp;

}

/**********************************************************
 * mm_realloc
 * Implemented in 3 cases
 *********************************************************/
void *mm_realloc(void *ptr, size_t size) {
    /* If size == 0 then this is just free, and we return NULL. */
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL) {
        return (mm_malloc(size));
    }
    //void *old_p = store_data(ptr); // malloc old data for error checking

    int oldsize = GET_SIZE(HDRP(ptr));
    /* Adjust block size to include overhead and alignment reqs. */
    int asize;
    if (size <= DSIZE) {
        asize = 2 * DSIZE;
    } else {
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }

    if (asize == oldsize) {
        // case 1: size unchanged, return current block directly

        /*if (!old_p) {
            check_realloc_consistency(old_p, ptr); 
         * // check the consistency of reallocated data
            free(old_p); // free debugging data to avoid memory leak
        }*/
        //mm_check();
        return ptr;
    } else if (asize > oldsize) {
        // case 2: size extended, unmalloc the current block
        PUT(HDRP(ptr), PACK(oldsize, 0));
        PUT(FTRP(ptr), PACK(oldsize, 0));
        // try coalescing current block and fit
        void *bp = coalesce(ptr);
        int block_size = GET_SIZE(HDRP(bp));
        if (block_size >= asize) {
            // case 2-1: if coalesced size suffices, maintain data first
            memmove(bp, ptr, oldsize - DSIZE);
            // later the same as find_fit
            int remainder_size = block_size - asize;
            if (remainder_size >= MIN_BLOCK_SIZE) {
                // the remainder is big enough as a new block
                void *newbp = (void *) bp + asize;
                PUT(HDRP(newbp), PACK(remainder_size, 0));
                PUT(FTRP(newbp), PACK(remainder_size, 0));
                list_insert(newbp);

                PUT(HDRP(bp), PACK(asize, 1));
                PUT(FTRP(bp), PACK(asize, 1));
            } else {
                PUT(HDRP(bp), PACK(block_size, 1));
                PUT(FTRP(bp), PACK(block_size, 1));
            }

            /*if (!old_p) {
                check_realloc_consistency(old_p, bp); 
             * // check the consistency of reallocated data
                free(old_p); // free debugging data to avoid memory leak
            }*/
            //mm_check();
            return (void *) bp;
        } else {
            // case 2-2: otherwise, alloc a new block
            void *newbp = find_fit(asize);
            if (newbp == NULL) {
                // case 2-2-1: there is no block available in the segregated lists, 
                // the heap must be extended
                if ((newbp = extend_heap(asize / WSIZE)) == NULL) return NULL;
                place(newbp, 1);
                // maintain data
                memmove(newbp, ptr, oldsize - DSIZE);
                // and free the old block
                list_insert((FreeBlock *) bp);

                /*if (!old_p) {
                    check_realloc_consistency(old_p, newbp); 
                 * // check the consistency of reallocated data
                    free(old_p); // free debugging data to avoid memory leak
                }*/
                //mm_check();
                return newbp;
            }
            // case 2-2-2: there is a block available in the segregated lists
            PUT(HDRP(newbp), PACK(asize, 1));
            PUT(FTRP(newbp), PACK(asize, 1));
            // maintain data
            memmove(newbp, ptr, oldsize - DSIZE);
            // and free the old block
            list_insert((FreeBlock *) bp);

            /*if (!old_p) {
                check_realloc_consistency(old_p, newbp); 
             * // check the consistency of reallocated data
                free(old_p); // free debugging data to avoid memory leak
            }*/
            //mm_check();
            return newbp;
        }
    } else {
        // case 3: size shrunk, same as find_fit
        int remainder_size = oldsize - asize;
        if (remainder_size >= MIN_BLOCK_SIZE) {
            // the remainder should be considered a new block
            void *newbp = (void *) ptr + asize;
            PUT(HDRP(newbp), PACK(remainder_size, 0));
            PUT(FTRP(newbp), PACK(remainder_size, 0));
            list_insert(newbp);

            PUT(HDRP(ptr), PACK(asize, 1));
            PUT(FTRP(ptr), PACK(asize, 1));
        }

        /*if (!old_p) {
            check_realloc_consistency(old_p, ptr); 
         * // check the consistency of reallocated data
            free(old_p); // free debugging data to avoid memory leak
        }*/
        //mm_check();
        return ptr;
    }
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check() {
    return check_coalesce() && check_fb_property();
}

/**********************************************************
 *  before checking the realloc data consistency, 
 *  we need to copy it to a safe place for future use
 *********************************************************/
void *store_data(void *old_p) {
    int block_size = GET_SIZE(HDRP(old_p)); // first get the size of the block pointed by the old pointer
    void *alloc_data = (void *) malloc(block_size * WSIZE);
    if (!alloc_data) {
        printf("NULL malloc\n");
        return NULL;
    }
    memmove(alloc_data, HDRP(old_p), block_size * WSIZE);
    return (void *) ((char *) alloc_data + WSIZE);
}
/*********************************************************
 * @param: void *old_p, pointing to the original data
 *         void *new_p, pointing to the reallocated data 
 * @return: 1 if success. 0 for failure
 * check after reallocation, if the new data is consistent with the old data;
 *********************************************************/ 
int check_realloc_consistency(void *old_p, void *new_p) {
    // first get the size of the block pointed by the new pointer
    int block_size = (GET_SIZE(HDRP(new_p)) < GET_SIZE(HDRP(old_p)) ? GET_SIZE(HDRP(new_p)) : GET_SIZE(HDRP(old_p)));
    if (block_size == 0) {
        return 1;
    }
    block_size = block_size - 2; // make sure we don't check the header and footer
    if (memcmp(old_p, new_p, block_size * WSIZE) != 0) { // memcmp returns 0 if data are the same
        /*for (int i = 0; i < block_size; i++) {
            if (((int *)old_p)[i] != ((int *)new_p)[i]) {
                printf("i: %d, new_p: %x, old_p: %x\n", i, ((int *)new_p)[i], ((int *)old_p)[i]);
                return 0; 
            }
        }
        printf("reallocated data is not consistent with the original data \n");
         */return 0;
    }
    return 1;
}

/********************************************************* 
 * @return: 1 if success. 0 for failure
 * check if all free blocks are coalesced properly;
 *********************************************************/ 
int check_coalesce() {
    void *bp; //pointer pointing to the beginning block in heap

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        // goes through entire heap to find all the free blocks
        if (FIND_LAST_BITH(bp) == 0 && FIND_LAST_BITF(bp) == 0) {
            // if the last bit of head and tail is 0, it is a free block
            if (((FIND_LAST_BITH(PREV_BLKP(bp)) == 0) && (bp != PREV_BLKP(bp))) 
                    // bp should not be the only block
                    || ((FIND_LAST_BITH(NEXT_BLKP(bp)) == 0) && (bp != NEXT_BLKP(bp)))) {
                // see if the last or the next block in the heap is also a free block
                // printf("bp: %p, prev bp: %p, next bp: %p\n", bp, PREV_BLKP(bp), NEXT_BLKP(bp));
                // printf("bp: %d, prev bp: %d, next bp: %d\n", FIND_LAST_BITH(bp), FIND_LAST_BITH(PREV_BLKP(bp)), FIND_LAST_BITF(NEXT_BLKP(bp)));
                printf("a free block in the heap is not coalesced properly \n");
                return 0;
            }
        }
    }
    return 1;
}

/********************************************************* 
 * @return: 1 if success. 0 for failure
 * 1. check the last bit of the header/footer of a free block is 0;
 * 2. check if all free blocks are in the lists;
 * 3. check if any of the free blocks are overlapped with each other in the heap;
 *********************************************************/
int check_fb_property() {
    FreeBlock *head;
    void *bp; // pointer pointing to the beginning block in heap

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        // goes through entire heap to find all the free blocks
        if (GET(HDRP(bp)) != GET(FTRP(bp))) {
            printf("header and footer of a block are not consistent \n");
            return 0;
        } else if (FIND_LAST_BITH(bp) == 0) {
            // if the last bit of head and tail is 0, it is a free block
            int found = 0;
            // found changes to 1 if the free block is found in a list
            for (int index = 0; index < NUM_SEG_LISTS; index++) {
                if (seg_lists[index] == NULL) continue;
                // make sure to check every free list there is
                head = seg_lists[index];
                do {
                    if (bp == head) {
                        found = 1;
                        continue;
                    }
                    // goes through the entire free block list
                    // to see if this free block is in the list
                    if ((bp > (void *) HDRP(head)) && (bp < (void *) FTRP(head))) {
                        // check if this free block overlaps with any other 
                        // free block within the free block list
                        printf("this free block overlaps with another free block in the list \n");
                        return 0;
                    }
                    head = head->next;
                } while (head != seg_lists[index]);
            }
            if (found == 0) {
                printf("a free block in the heap is not found in the list \n");
                return 0;
            }
        }
    }
    return 1;
}
