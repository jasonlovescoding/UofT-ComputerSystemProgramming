CC = gcc
CC_FLAGS = -O3 -DNDEBUG
CC_DBG_FLAGS = -g
INCLUDES = ../include

# set environment variable TOPDIR to the top-level directory where
# the code for this assignment is located.
#TOPDIR=$(HOME)/469/a2

all: libmmutil

debug: libmmutil_dbg

# Optimized versions

mm_thread.o: mm_thread.c $(INCLUDES)/mm_thread.h
	$(CC) $(CC_FLAGS) -c -I$(INCLUDES) mm_thread.c

tsc.o: tsc.c $(INCLUDES)/tsc.h	
	$(CC) $(CC_FLAGS) -c -I$(INCLUDES) tsc.c

memlib.o: memlib.c $(INCLUDES)/memlib.h
	$(CC) $(CC_FLAGS) -c -I$(INCLUDES) memlib.c

libmmutil: memlib.o tsc.o mm_thread.o
	ar rs libmmutil.a memlib.o tsc.o mm_thread.o

# Debugging versions

mm_thread_dbg.o: mm_thread.c $(INCLUDES)/mm_thread.h
	$(CC) $(CC_DBG_FLAGS) -c -o $(@) -I$(INCLUDES) mm_thread.c

tsc_dbg.o: tsc.c $(INCLUDES)/tsc.h	
	$(CC) $(CC_DBG_FLAGS) -c -o $(@) -I$(INCLUDES) tsc.c

memlib_dbg.o: memlib.c $(INCLUDES)/memlib.h
	$(CC) $(CC_DBG_FLAGS) -c -o $(@) -I$(INCLUDES) memlib.c

libmmutil_dbg: memlib_dbg.o tsc_dbg.o mm_thread_dbg.o
	ar rs libmmutil_dbg.a memlib_dbg.o tsc_dbg.o mm_thread_dbg.o

clean:
	rm -f *.o *.a *~
