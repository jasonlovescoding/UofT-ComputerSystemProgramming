CC = gcc
CFLAGS =  -Wall -O1 -g

OBJS = mdriver.o mm.o memlib.o fsecs.o fcyc.o clock.o ftimer.o

mdriver: $(OBJS)
	$(CC) $(CFLAGS) -o mdriver $(OBJS)

mm.o: mm.c mm.h memlib.h

clean:
	rm -f *~ mm.o mdriver


