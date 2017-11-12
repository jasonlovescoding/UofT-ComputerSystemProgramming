/* $Id$ */

/*
 *  CSC469 - Assignment 2
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>

#include "memlib.h"


char *dseg_lo = NULL, *dseg_hi = NULL;
long dseg_size;  /* Maximum size of data segment */

static int page_size;

/* Align pointer to closest page boundary downwards */
#define PAGE_ALIGN(p)    ((void *)(((unsigned long)(p) / page_size) * page_size))
/* Align pointer to closest page boundary upwards */
#define PAGE_ALIGN_UP(p) ((void *)((((unsigned long)(p) + page_size - 1) / page_size) * page_size))



int mem_init (void)
{

    /* Get system page size */
    page_size = (int) getpagesize();

    /* Allocate heap */
    dseg_lo = (char *) malloc(DSEG_MAX + 2*page_size);
    if (!dseg_lo)
        return -1;

    /* align heap to the next page boundary */
    dseg_lo = (char *) PAGE_ALIGN_UP(dseg_lo);
    dseg_hi = dseg_lo-1;
    dseg_size = DSEG_MAX;


    return 0;
}


void *mem_sbrk (ptrdiff_t increment)
{
    char *new_hi = dseg_hi + increment;
    char *old_hi = dseg_hi;
    long dseg_cursize = dseg_hi - dseg_lo + 1;

    assert(increment > 0);

    /* Resize data segment, if the memory is available */
    if (new_hi > dseg_lo + dseg_size)
        return NULL;
    dseg_hi = new_hi;
    dseg_cursize = dseg_hi - dseg_lo + 1;

    return (void *)(old_hi + 1);
}

int mem_pagesize (void)
{
    return page_size;
}

ptrdiff_t mem_usage (void)
{
  /* hack for libc */
  if (dseg_lo != NULL && dseg_hi == NULL) {
    dseg_hi = sbrk(0);
  }
  return dseg_hi - dseg_lo;
}
 
