#include <sys/types.h>

/* initialize the cycle counter */
u_int64_t start = 0;


/* Set *hi and *lo to the high and low order bits of the cycle counter.
 * Implementation requires assembly code to use the rdtsc instruction.
 */
static inline void access_counter(unsigned *hi, unsigned *lo)
{
  asm volatile("rdtsc; movl %%edx, %0; movl %%eax, %1" /* Read cycle counter */
      : "=r" (*hi), "=r" (*lo)                /* and move results to */
      : /* No input */                        /* the two outputs */
      : "%edx", "%eax");
}

void start_counter()
{
  unsigned hi, lo;
  access_counter(&hi, &lo);
  start = ((u_int64_t)hi << 32) | lo;
}

u_int64_t get_counter()
{
  unsigned ncyc_hi, ncyc_lo;

  /* Get the cycle counter */
  access_counter(&ncyc_hi, &ncyc_lo);

  return (((u_int64_t)ncyc_hi << 32) | ncyc_lo) - start;

}

