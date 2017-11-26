#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "random_bit.h"

static int
to_int (int* num, const char* s)
{
  errno = 0;
  *num = strtol (s, NULL, 10);
  if (errno != 0)
    {
      errno = 0; /* Reset errno */
      return -1;
    }
  else
    return 0;
}

static void
print_usage (const char argv0[])
{
  fprintf (stderr, "Usage: %s <number of rows> <number of columns>\n\n", argv0);
}

int 
main (int argc, char* argv[]) 
{
  const int verbose = 0;
  int ii, jj, nrows, ncols;
  int err = 0;

  init_random_bit (get_random_seed ());

  if (argc != 3) 
    {
      fprintf (stderr, "Incorrect number of command-line arguments!\n");
      print_usage (argv[0]);
      exit (EXIT_FAILURE);
    }

  err = to_int (&nrows, argv[1]);
  if (err != 0)
    {
      fprintf (stderr, "Command-line argument %s is not an integer!\n", argv[1]);
      print_usage (argv[0]);
      exit (EXIT_FAILURE);
    }
  else if (nrows < 1)
    {
      fprintf (stderr, "Command-line argument nrows = %d should be a positive integer!\n", nrows);
      print_usage (argv[0]);
      exit (EXIT_FAILURE);
    }

  err = to_int (&ncols, argv[2]);
  if (err != 0)
    {
      fprintf (stderr, "Command-line argument %s is not an integer!\n", argv[2]);
      print_usage (argv[0]);
      exit (EXIT_FAILURE);
    }
  else if (nrows < 1)
    {
      fprintf (stderr, "Command-line argument ncols = %d should be a positive integer!\n", ncols);
      print_usage (argv[0]);
      exit (EXIT_FAILURE);
    }

  if (verbose)
    printf ("Number of rows: %d\n"
	    "Number of cols: %d\n", 
	    nrows, ncols);

  printf ("P1\n%d %d\n", nrows, ncols);
  for (ii = 0; ii < nrows; ii++) 
    for (jj = 0; jj < ncols; jj++) 
      printf ("%c\n", '0' + random_bit ());
  
  return 0;
}
