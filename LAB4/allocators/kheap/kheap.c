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
     "TeamBart",
     /* Full name of first team member */
     "Bartholomew JoJo Simpson",
     /* Email address of first team member */
     "BartSimpson@mail.utoronto.ca",
     /* Student Number of first team member */
     "998111111"
     /* Full name of second team member */
     "",
     /* Email address of second team member */
     "",
     /* Student Number of second team member */
     ""
};


typedef unsigned long vaddr_t;

static
void
fill_deadbeef(void *vptr, size_t len)
{
	u_int32_t *ptr = vptr;
	size_t i;

	for (i=0; i<len/sizeof(u_int32_t); i++) {
		ptr[i] = 0xdeadbeef;
	}
}

////////////////////////////////////////////////////////////
//
// Pool-based subpage allocator.
//
// It works like this:
//
//    We allocate one page at a time and fill it with objects of size k,
//    for various k. Each page has its own freelist, maintained by a
//    linked list in the first word of each object. Each page also has a
//    freecount, so we know when the page is completely free and can 
//    recycle it.  We never return pages to the system, we just keep a 
//    pool of completely empty pages that can be reused for a different 
//    size.
//
//    No assumptions are made about the sizes k; they need not be
//    powers of two (although in this implementation they are). 
//    Note, however, that malloc must always return
//    pointers aligned to the maximum alignment requirements of the
//    platform; thus block sizes must at least be multiples of 8.
//    They must also be at least sizeof(struct freelist).
//
//    The free counts and addresses of the pages are maintained in
//    another list.  Maintaining this table is a nuisance, because it
//    cannot recursively use the subpage allocator. (We could probably
//    make that work, but it would be painful.)
//

#undef  SLOW	/* consistency checks */
#undef SLOWER	/* lots of consistency checks */

////////////////////////////////////////


#define PAGE_SIZE  4096
#define PAGE_FRAME 0xfffff000

#define NSIZES 9
static const size_t sizes[NSIZES] = { 8, 16, 32, 64, 128, 256, 512, 1024, 2048 };

#define SMALLEST_SUBPAGE_SIZE 8
#define LARGEST_SUBPAGE_SIZE 2048

////////////////////////////////////////

struct freelist {
	struct freelist *next;
};

struct pageref {
	struct pageref *next;
	struct freelist *flist;
	vaddr_t pageaddr_and_blocktype;
	int nfree;
};

struct big_freelist {
	int npages;
	struct big_freelist *next;
};

#define INVALID_OFFSET   (0xffff)

#define PR_PAGEADDR(pr)  ((pr)->pageaddr_and_blocktype & PAGE_FRAME)
#define PR_BLOCKTYPE(pr) ((pr)->pageaddr_and_blocktype & ~PAGE_FRAME)
#define MKPAB(pa, blk)   (((pa)&PAGE_FRAME) | ((blk) & ~PAGE_FRAME))

////////////////////////////////////////

/*
 * Unlike the OS/161 kheap, we can't just assign one page of 
 * pagerefs.  We need to be able to expand the size of the 
 * heap as demanded by the application.  
 *
 * Instead, we maintain several linked lists of pagerefs:
 * fresh_refs == never used, must obtain a page of memory
 *               along with the pageref
 * recycled_refs == pagerefs that refer to a completely empty
 *                  page of memory
 * sizebases == array of lists of pagerefs; each entry corresponds
 *              to a particular object size, and holds the list
 *              of in-use pagerefs for that size.
 *
 * We also have a special list for large allocations.
 */

static struct pageref *fresh_refs; /* static global, initially 0 */
static struct pageref *recycled_refs;
static struct pageref *sizebases[NSIZES];
static struct big_freelist *bigchunks;

static
struct pageref *
allocpageref(void)
{
	struct pageref *ref;

	/* Use a pageref that already has a page allocated,
	 * if there are any.
	 */
	if (recycled_refs) {
		ref = recycled_refs;
		recycled_refs = recycled_refs->next;
		return ref;
	}

	/* No recycled_refs, use fresh one, if there are any */
	if (fresh_refs) {
		ref = fresh_refs;
		fresh_refs = fresh_refs->next;
		return ref;
	}

	/* All out of pagerefs.  Initialize a new page worth by
	 * getting a page with mem_sbrk()
	 */

	ref = (struct pageref *)mem_sbrk(PAGE_SIZE);
	if (ref) {
		bzero(ref, PAGE_SIZE);
		fresh_refs = ref+1;
		struct pageref *tmp = fresh_refs;
		int nrefs = PAGE_SIZE / sizeof(struct pageref) - 1;
		int i;
		for (i = 0; i < nrefs-1; i++) {
			tmp->next = tmp+1;
			tmp = tmp->next;
		}
		tmp->next = NULL;
	}
	return ref;

}

static
void
freepageref(struct pageref *p)
{
	p->next = recycled_refs;
	recycled_refs = p;
}


////////////////////////////////////////

/* SLOWER implies SLOW */
#ifdef SLOWER
#ifndef SLOW
#define SLOW
#endif
#endif

#ifdef SLOW
static
void
checksubpage(struct pageref *pr)
{
	vaddr_t prpage, fla;
	struct freelist *fl;
	int blktype;
	int nfree=0;

	if (pr->flist == NULL) {
		assert(pr->nfree==0);
		return;
	}

	prpage = PR_PAGEADDR(pr);
	blktype = PR_BLOCKTYPE(pr);
	

	for (fl=pr->flist; fl != NULL; fl = fl->next) {
		fla = (vaddr_t)fl;
		assert(fla >= prpage && fla < prpage + PAGE_SIZE);
		assert((fla-prpage) % sizes[blktype] == 0);
		nfree++;
	}
	assert(nfree==pr->nfree);
}
#else
#define checksubpage(pr) ((void)(pr))
#endif

#ifdef SLOWER
static
void
checksubpages(void)
{
	struct pageref *pr;
	int i;
	unsigned sc=0;

	for (i=0; i<NSIZES; i++) {
		for (pr = sizebases[i]; pr != NULL; pr = pr->next) {
			checksubpage(pr);
			sc++;
		}
	}

}
#else
#define checksubpages() 
#endif

////////////////////////////////////////

static
void
remove_lists(struct pageref *pr, int blktype)
{
	struct pageref **guy;

	assert(blktype>=0 && blktype<NSIZES);

	for (guy = &sizebases[blktype]; *guy; guy = &(*guy)->next) {
		checksubpage(*guy);
		if (*guy == pr) {
			*guy = pr->next;
			break;
		}
	}

}

static
inline
int blocktype(size_t sz)
{
	unsigned i;
	for (i=0; i<NSIZES; i++) {
		if (sz <= sizes[i]) {
			return i;
		}
	}

	printf("Subpage allocator cannot handle allocation of size %lu\n", 
	      (unsigned long)sz);
	exit(1);

	// keep compiler happy
	return 0;
}

static
void *
subpage_kmalloc(size_t sz)
{
	unsigned blktype;	// index into sizes[] that we're using
	struct pageref *pr;	// pageref for page we're allocating from
	vaddr_t prpage;		// PR_PAGEADDR(pr)
	vaddr_t fla;		// free list entry address
	struct freelist *fl;	// free list entry
	void *retptr;		// our result

	volatile int i;


	blktype = blocktype(sz);
	sz = sizes[blktype];


	checksubpages();

	for (pr = sizebases[blktype]; pr != NULL; pr = pr->next) {

		/* check for corruption */
		assert(PR_BLOCKTYPE(pr) == blktype);
		checksubpage(pr);

		if (pr->nfree > 0) {

		doalloc: /* comes here after getting a whole fresh page */

			prpage = PR_PAGEADDR(pr);
			fl = pr->flist;

			retptr = pr->flist;
			pr->flist = pr->flist->next;
			pr->nfree--;

			if (pr->flist != NULL) {
				assert(pr->nfree > 0);
				fla = (vaddr_t)fl;
				assert(fla - prpage < PAGE_SIZE);
			}
			else {
				assert(pr->nfree == 0);
			}

			checksubpages();
			return retptr;
		}
	}

	/*
	 * No page of the right size available.
	 * Make a new one.
	 */

	pr = allocpageref();
	if (pr==NULL) {
		/* Couldn't allocate accounting space for the new page. */
		printf("malloc: Subpage allocator couldn't get pageref\n"); 
		return NULL;
	}

	prpage = PR_PAGEADDR(pr);
	if (prpage == 0) {
		prpage = (vaddr_t)mem_sbrk(PAGE_SIZE);
		if (prpage==0) {
			/* Out of memory. */
			freepageref(pr);
			printf("malloc: Subpage allocator couldn't get a page\n"); 
			return NULL;
		}
	}

	pr->pageaddr_and_blocktype = MKPAB(prpage, blktype);
	pr->nfree = PAGE_SIZE / sizes[blktype];

	/* Build freelist */

	fla = prpage;
	fl = (struct freelist *)fla;
	fl->next = NULL;
	for (i=1; i<pr->nfree; i++) {
		fl = (struct freelist *)(fla + i*sizes[blktype]);
		fl->next = (struct freelist *)(fla + (i-1)*sizes[blktype]);
		assert(fl != fl->next);
	}
	pr->flist = fl;
	assert((vaddr_t)pr->flist == prpage+(pr->nfree-1)*sizes[blktype]);

	pr->next = sizebases[blktype];
	sizebases[blktype] = pr;


	/* This is kind of cheesy, but avoids duplicating the alloc code. */
	goto doalloc;
}

static
int
subpage_kfree(void *ptr)
{
	int blktype;		// index into sizes[] that we're using
	vaddr_t ptraddr;	// same as ptr
	struct pageref *pr=NULL;// pageref for page we're freeing in
	vaddr_t prpage;		// PR_PAGEADDR(pr)
	vaddr_t offset;		// offset into page
	int i;

	ptraddr = (vaddr_t)ptr;

	checksubpages();

	/* Nasty search to find the page that this block came from */

	for (i=0; i < NSIZES && pr==NULL; i++) {
		for (pr = sizebases[i]; pr; pr = pr->next) {
			prpage = PR_PAGEADDR(pr);
			blktype = PR_BLOCKTYPE(pr);

			/* check for corruption */
			assert(blktype>=0 && blktype<NSIZES);
			checksubpage(pr);

			if (ptraddr >= prpage && ptraddr < prpage + PAGE_SIZE) {
				break;
			}
		}
	}

	if (pr==NULL) {
		/* Not on any of our pages - not a subpage allocation */
		return -1;
	}

	offset = ptraddr - prpage;

	/* Check for proper positioning and alignment */
	if (offset >= PAGE_SIZE || offset % sizes[blktype] != 0) {
		printf("kfree: subpage free of invalid addr %p\n", ptr);
		exit(1);
	}

	/*
	 * Clear the block to 0xdeadbeef to make it easier to detect
	 * uses of dangling pointers.
	 */
	fill_deadbeef(ptr, sizes[blktype]);

	/*
	 * We probably ought to check for free twice by seeing if the block
	 * is already on the free list. But that's expensive, so we don't.
	 */
	((struct freelist *)ptr)->next = pr->flist;
	pr->flist = (struct freelist *)ptr;
	pr->nfree++;

	assert(pr->nfree <= PAGE_SIZE / sizes[blktype]);
	if (pr->nfree == PAGE_SIZE / sizes[blktype]) {
		/* Whole page is free. */
		remove_lists(pr, blktype);
		freepageref(pr);
	}

	checksubpages();

	return 0;
}

static void *big_kmalloc(int sz)
{
	/* Handle requests bigger than LARGEST_SUBPAGE_SIZE 
	 * We simply round up to the nearest page-sized multiple
	 * after adding some overhead space to hold the number of 
	 * pages.
	 */
	
	void *result = NULL;

	sz += SMALLEST_SUBPAGE_SIZE;
	/* Round up to a whole number of pages. */
	int npages = (sz + PAGE_SIZE - 1)/PAGE_SIZE;

	/* Check if we happen to have a chunk of the right size already */
	struct big_freelist *tmp = bigchunks;
	struct big_freelist *prev = NULL;
	while (tmp != NULL) {
		if (tmp->npages > npages) {
			/* Carve the block in two pieces */
			tmp->npages -= npages;
			int *hdr_ptr = (int *)((char *)tmp+(tmp->npages*PAGE_SIZE));
			*hdr_ptr = npages;
			result = (void *)((char *)hdr_ptr + SMALLEST_SUBPAGE_SIZE);
			break;
		} else if (tmp->npages == npages) {
			/* Remove block from freelist */
			if (prev) {
				prev->next = tmp->next;
			} else {
				bigchunks = tmp->next;
			}
			int *hdr_ptr = (int *)tmp;
			assert(*hdr_ptr == npages);
			result = (void *)((char *)hdr_ptr + SMALLEST_SUBPAGE_SIZE);
			break;
		} else {
			prev = tmp;
			tmp = tmp->next;
		}
	}

	if (result == NULL) {
		/* Nothing suitable in freelist... grab space with mem_sbrk */
		int *hdr_ptr = (int *)mem_sbrk(npages*PAGE_SIZE);
		if (hdr_ptr != NULL) {
			*hdr_ptr = npages;
			result = (void *)((char *)hdr_ptr + SMALLEST_SUBPAGE_SIZE);
		}
	}

	return result;
}

static void big_kfree(void *ptr)
{
	/* Coalescing is unlikely to do much good (other page allocations
	 * for small objects are likely to prevent big chunks from fitting
	 * together), so we don't bother trying.
	 */

	int *hdr_ptr = (int *)((char *)ptr - SMALLEST_SUBPAGE_SIZE);
	//int npages = *hdr_ptr;

	struct big_freelist *newfree = (struct big_freelist *) hdr_ptr;
	assert(newfree->npages == *hdr_ptr);
	newfree->next = bigchunks;
	bigchunks = newfree;
}

//
////////////////////////////////////////////////////////////

pthread_mutex_t malloc_lock = PTHREAD_MUTEX_INITIALIZER;

int mm_init(void)
{
	if (dseg_lo == NULL && dseg_hi == NULL) {
		return mem_init();
	}
	return 0;
}

void *
mm_malloc(size_t sz)
{
	void *result;

	pthread_mutex_lock(&malloc_lock);

	if (sz>=LARGEST_SUBPAGE_SIZE) {
		result = big_kmalloc(sz);
	} else {
		result = subpage_kmalloc(sz);
	}

	pthread_mutex_unlock(&malloc_lock);

	return result;
}

void
mm_free(void *ptr)
{
	/*
	 * Try subpage first; if that fails, assume it's a big allocation.
	 */
	if (ptr == NULL) {
		return;
	} else {
	  pthread_mutex_lock(&malloc_lock);
	  if (subpage_kfree(ptr)) {
		  big_kfree(ptr);
	  }
	  pthread_mutex_unlock(&malloc_lock);
	}
}

