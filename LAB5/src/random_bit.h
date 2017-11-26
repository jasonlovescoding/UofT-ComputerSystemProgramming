#ifndef _random_bit_h
#define _random_bit_h
#include <inttypes.h>

/**
 * Initializes the random-bit number generator.  Please only call this
 * in one thread!
 */
void
init_random_bit (uint32_t seed);

/**
 * Returns a "random bit" (a character which is either 0 or 1).  Note:
 * the results are NOT in {'0', '1'}, but are in {0, 1}.  These are two
 * different things!  To generate ASCII '0' or '1', just add '0' to the
 * result.
 * 
 * @note Only call in one thread!
 */
char
random_bit ();

/**
 * Return a 32-bit unsigned random seed.
 */
uint32_t
get_random_seed ();

#endif /* _random_bit_h */

