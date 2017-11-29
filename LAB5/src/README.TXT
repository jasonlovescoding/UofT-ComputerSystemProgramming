
[111214]

- initboard may appear to hang from time to time. This is because it tries to
  get a good random seed by reading from /dev/random, which may take a long
  time due to the lack of random information accumulated in the system (e.g.,
  when the machine has just been rebooted). Since randomness is not the focus
  of this homework, the program has been modified to rely on /dev/urandom (as
  opposed to /dev/random), which is consistently fast. See random_bit.c.

- Given N iterations at the command line, the current game_of_life routine
  actually returns the board after N-1 iterations, even though it computes the
  N'th iteration. In order to avoid confusing students, the routine has been
  updated to return the correct board (i.e. after N iterations).


#define UNROLL( __i, __isouth ) do {\
				jnw = jw; jn = jc; jjn = jjc; jjne = jje;\
				jw = jsw; jc = js; jjc = jjs; jje = jjse;\
				jsw = BOARD(inboard, __isouth, jwest);\
				js = BOARD(inboard, __isouth, j);\
				jjs = BOARD(inboard, __isouth, jj);\
				jjse = BOARD(inboard, __isouth, jjeast);\
				BOARD(outboard, __i, j) = alivep(jnw + jn + jjn + jw + jjc + jsw + js + jjs, jc); \
				BOARD(outboard, __i, jj) = alivep(jn + jjn + jjne + jc + jje + js + jjs + jjse, jjc); \
} while(0)
