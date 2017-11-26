#ifndef _save_h 
#define _save_h

#include <stdio.h>

/**
 * To the file stream "output", save the dimensions (nrows and ncols,
 * respectively) of the board, and then save the contents of the board
 * (assuming that it's stored in column-major order).
 */
void
save_board (FILE* output, const char board[], const int nrows, const int ncols);

#endif /* _load_h */
