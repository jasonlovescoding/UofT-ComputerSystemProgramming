#
# Include platform-dependent settings, and definitions.
#
include Makefile.include

LDFLAGS += -lm -pthread
CPPFLAGS += "-DCOMPILER=\"$(CC)\"" "-DFLAGS=\"$(COPTFLAGS)\""

#
# Include the realtime clock library, if required.
#
ifeq ($(TIMER_TYPE),USE_CLOCK_GETTIME_REALTIME)
	LDFLAGS += -lrt
else
	ifeq ($(TIMER_TYPE),USE_CLOCK_GETTIME_MONOTONIC)
		LDFLAGS += -lrt
	endif
endif

GOL_EXE = gol
GOL_VERIFY_EXE = gol_verify
GOL_OBJS = gol.o barrier.o life.o lifeseq.o load.o save.o 
GOL_VERIFY_OBJS = gol.verify.o life.o lifeseq.o load.o save.o 
BITBOARD_EXE = initboard
BITBOARD_OBJS = bitboard.o random_bit.o
EXES = $(GOL_EXE) $(BITBOARD_EXE)
OBJS = $(GOL_OBJS) $(BITBOARD_OBJS)


all: $(GOL_EXE) $(BITBOARD_EXE)

$(GOL_EXE): $(GOL_OBJS) 
	$(LINKER) $(CFLAGS) $(LDFLAGS) $(GOL_OBJS) -o $@ 

$(GOL_VERIFY_EXE): $(GOL_VERIFY_OBJS) 
	$(LINKER) $(CFLAGS) $(LDFLAGS) $(GOL_VERIFY_OBJS) -o $@ 

$(BITBOARD_EXE): $(BITBOARD_OBJS)
	$(LINKER) $(CFLAGS) $(LDFLAGS) $(BITBOARD_OBJS) -o $@ 

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

%.verify.o: %.c
	$(CC) -c $(CFLAGS) -DVERIFY_FLAG $(CPPFLAGS) $< -o $@

barrier.o: barrier.h barrier.c

lifeseq.o: lifeseq.c life.h util.h

life.o: life.c life.h util.h 

load.o: load.c load.h

save.o: save.c save.h

gol.o: gol.c life.h load.h save.h 

bitboard.o: bitboard.c random_bit.h

random_bit.o: random_bit.c random_bit.h


clean:
	rm -f $(GOL_OBJS) $(GOL_VERIFY_OBJS) $(GOL_EXE) $(GOL_VERIFY_EXE) $(BITBOARD_OBJS) $(BITBOARD_EXE) 
