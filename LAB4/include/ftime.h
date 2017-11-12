/* Function timer */

extern "C" void init_itimers(void);
extern "C" double get_utime(void);
extern "C" double get_ustime(void);
extern "C" double get_rtime(void);

typedef void (*test_funct)(void); 
double ftime(test_funct P, double E);


