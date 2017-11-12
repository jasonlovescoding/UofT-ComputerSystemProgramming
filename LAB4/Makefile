BENCHDIR := benchmarks
DIRS := cache-scratch cache-thrash larson threadtest #linux-scalability

all:
	cd util; make
	cd allocators; make
	for dir in $(DIRS); do \
	  (cd $(BENCHDIR)/$$dir; ${MAKE}); \
	done

debug:
	cd util; make debug
	cd allocators; make debug
	for dir in $(DIRS); do \
	  (cd $(BENCHDIR)/$$dir; ${MAKE} debug); \
	done

clean:
	cd util; make clean
	cd allocators; make clean
	for dir in $(DIRS); do \
	  (cd $(BENCHDIR)/$$dir; ${MAKE} clean); \
	done

