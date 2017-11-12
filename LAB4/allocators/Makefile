CC = gcc

CC_FLAGS = -std=gnu99 -c -Wall -fmessage-length=0 -pipe -O3 -finline-limit=65000 -fkeep-inline-functions -finline-functions -ffast-math -fomit-frame-pointer -DNDEBUG -I. -I$(TOPDIR)/include -D_REENTRANT=1

CC_DBG_FLAGS = -c -Wall -fmessage-length=0 -pipe -g -I. -I$(TOPDIR)/include -D_REENTRANT=1

all: libkheap libmmlibc liballoc

debug: libkheap_dbg libmmlibc_dbg liballoc_dbg

alloclibs:
	mkdir alloclibs

# Library containing mm_malloc and mm_free for OS/161 kheap-based allocator

libkheap: alloclibs
	cd kheap; $(CC) $(CC_FLAGS) kheap.c; ar rs ../alloclibs/libkheap.a kheap.o

libkheap_dbg: alloclibs
	cd kheap; $(CC) $(CC_DBG_FLAGS) kheap.c; ar rs ../alloclibs/libkheap_dbg.a kheap.o 

# Library containing mm_malloc and mm_free for lab 4
liballoc: alloclibs
	cd alloc; $(CC) $(CC_FLAGS) alloc.c; ar rs ../alloclibs/liballoc.a alloc.o

liballoc_dbg: alloclibs
	cd alloc; $(CC) $(CC_DBG_FLAGS) alloc.c; ar rs ../alloclibs/liballoc_dbg.a alloc.o 

# Library containing mm_malloc and mm_free wrappers for libc allocator
libmmlibc: alloclibs
	cd libc; $(CC) $(CC_FLAGS) libc_wrapper.c; ar rs ../alloclibs/libmmlibc.a libc_wrapper.o

libmmlibc_dbg: alloclibs
	cd libc; $(CC) $(CC_DBG_FLAGS) libc_wrapper.c; ar rs ../alloclibs/libmmlibc_dbg.a libc_wrapper.o

clean:
	rm -rf alloclibs; rm -f */*.o; rm -f *~
