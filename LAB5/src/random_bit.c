#include "random_bit.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


uint32_t
get_random_seed ()
{
  const int nread = 4;
  FILE* r = NULL;
  uint32_t seed = (uint32_t) 0;
  char buffer[4];

  assert ((size_t) nread == sizeof (uint32_t));
  r = fopen ("/dev/urandom", "r");
  if (r == NULL)
    {
      time_t current_time;
      fprintf (stderr, "*** Warning: Failed to open /dev/random, "
               "falling back on system clock for random seed ***\n");
      current_time = time (NULL);
      return (uint32_t) current_time;
    }
  assert ((size_t) nread == fread (buffer, sizeof(char), (size_t) nread, r));
  seed = *((uint32_t*) buffer);
  fclose (r);
  return seed;
}


#ifdef USE_MKL_PRNG
#include <mkl_vsl.h>

static VSLStreamStatePtr stream = NULL;

static void
deinit_random_bit ()
{
  (void) vslDeleteStream (&stream);
  stream = NULL;
}

void
init_random_bit (uint32_t seed)
{
  assert (0 == vslNewStream (&stream, VSL_BRNG_MT19937, (unsigned int) seed));
  assert (0 == atexit (deinit_random_bit));
}

char
random_bit ()
{
  int errcode = 0;
  int i = -1; /* for error-checking */

  assert (0 == viRngUniform (VSL_METHOD_IUNIFORM_STD, stream, 1, &i, 0, 2));
  assert (errcode == 0);

  return (char) i;
}
#else /* use the system pseudorandom number generator to get random bits */

void
init_random_bit (uint32_t seed)
{
  srand ((unsigned int) seed);
}

char
random_bit ()
{
  /*
   * Hopefully this makes somewhat random bits.  Accuracy / quality
   * aren't so important for this application; we just want some
   * initial data that isn't strongly patterned.
   */
  const int r = rand () / (RAND_MAX / 2);
  return (char) r;
}
#endif /* USE_MKL_PRNG */







