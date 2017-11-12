#include <stdlib.h>
#include "memlib.h"

void *mm_malloc(size_t sz)
{
  return malloc(sz);
}

void mm_free(void *ptr)
{
  free(ptr);
}


int mm_init(void)
{
  dseg_lo = sbrk(0);
  return 0;
}
