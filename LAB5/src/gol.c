#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "life.h"
#include "load.h"
#include "save.h"

#ifdef VERIFY_FLAG
#define DO_VERIFY 1
#else // VERIFY_FLAG
#define DO_VERIFY 0
#endif // VERIFY_FLAG


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
  fprintf (stderr, 
	   "\nUsage: %s <num_generations> <infilename> <outfilename>\n\n"
	   "\t<num_generations>: nonnegative number of generations\n"
	   "\t<infilename>:      file from which to load initial board configuration\n"
	   "\t<outfilename>:     file to which to save final board configuration;\n"
           "\t                   if not provided or just a single hyphen (-), then \n"
	   "\t                   board is output to stdout\n"
	   "\n\n", argv0);
}

static void
copy_board (char outboard[], const char inboard[], const int nrows, const int ncols)
{
  /* We use memmove in case outboard and inboard overlap (in this
     case, this would mean that they are the same */
  memmove (outboard, inboard, nrows * ncols * sizeof (char));
}

static int
boards_equalp (const char b1[], const char b2[], const int nrows, const int ncols)
{
  int i;
  for (i = 0; i < nrows * ncols; i++)
    if (b1[i] != b2[i])
      return 0;

  return 1;
}

int 
main (int argc, char* argv[])
{
  /*
   * Set verifyp to 1 if you want to turn on verification.
   */
  const int verifyp = DO_VERIFY;
  const int argc_min = 3;
  const int argc_max = 4;

  int gens_max = 0;
  char* inboard = NULL;
  char* outboard = NULL;
  char* checkboard = NULL;
  char* final_board = NULL;
  int nrows = 0;
  int ncols = 0;
  FILE* input = NULL;
  FILE* output = NULL;
  int err = 0;

  /* Parse command-line arguments */
  if (argc < argc_min || argc > argc_max)
    {
      fprintf (stderr, "*** Wrong number of command-line arguments; "
	       "got %d, need at least %d and no more than %d ***\n", 
	       argc - 1, argc_min - 1, argc_max - 1);
      print_usage (argv[0]);
      exit (EXIT_FAILURE);
    }
  
  err = to_int (&gens_max, argv[1]);
  if (err != 0)
    {
      fprintf (stderr, "*** <num_generations> argument \'%s\' "
	       "must be a nonnegative integer! ***\n", argv[1]);
      print_usage (argv[0]);
      exit (EXIT_FAILURE);
    }

  /* Open input and output files */ 
  input = fopen (argv[2], "r");
  if (input == NULL)
    {
      fprintf (stderr, "*** Failed to open input file \'%s\' for reading! ***\n", argv[2]);
      print_usage (argv[0]);
      exit (EXIT_FAILURE);
    }

  if (argc < argc_max || 0 == strcmp (argv[3], "-"))
    output = stdout;
  else
    {
      output = fopen (argv[3], "w");
      if (output == NULL)
	{
	  fprintf (stderr, "*** Failed to open output file \'%s\' for writing! ***\n", argv[3]);
	  print_usage (argv[0]);
	  exit (EXIT_FAILURE);
	}
    }

  /* Load the initial board state from the input file */
  inboard = load_board (input, &nrows, &ncols);
  fclose (input);

  /* Create a second board for ping-ponging */
  outboard = make_board (nrows, ncols);

  /* If we want to verify the result, then make a third board and copy
     the initial state into it */
  if (verifyp)
    {
      checkboard = make_board (nrows, ncols);
      copy_board (checkboard, inboard, nrows, ncols);
    }

  /* 
   * Evolve board gens_max ticks, and time the evolution.  You will
   * parallelize the game_of_life() function for this assignment.
   */
  final_board = game_of_life (outboard, inboard, nrows, ncols, gens_max);

  /* Print (or save, depending on command-line argument <outfilename>)
     the final board */
  save_board (output, final_board, nrows, ncols);
  if (output != stdout && output != stderr)
    fclose (output);

  if (verifyp)
    {
      /* Make sure that outboard has the final state, so we can verify
	 it.  Since we ping-pong between inboard and outboard, it
	 could be either that inboard == final_board or that outboard
	 == final_board */
      copy_board (outboard, final_board, nrows, ncols);

      /* Ping-pong between checkboard (contains the initial state) and
	 inboard */
      final_board = sequential_game_of_life (inboard, checkboard, nrows, ncols, gens_max);

      if (boards_equalp (final_board, outboard, nrows, ncols))
	printf ("Verification successful\n");
      else
	{
	  fprintf (stderr, "*** Verification failed! ***\n");
	  exit (EXIT_FAILURE);
	}
    }

  /* Clean up */
  if (inboard != NULL)
    free (inboard);
  if (outboard != NULL)
    free (outboard);
  if (checkboard != NULL)
    free (checkboard);

  return 0;
}

