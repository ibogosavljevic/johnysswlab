
CC=g++
OPT?=3
DEPS= 
LDFLAGS+=-lpapi
FUNC_OPT?=0
CFLAGS+=-I. -std=c++11 -O$(OPT) -pthread -g -Werror $(RPATH) -DFUNC_OPT=$(FUNC_OPT) -DHAS_PAPI


WORKING_SET=10000 100000 1000000 10000000 100000000
NUMBER_OF_SEARCHES=10000000
PERF=perf stat -B -e cache-references,cache-misses,cache-misses:u,cycles,instructions,branches,branch-misses,faults,migrations 

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS)
	$(CC) -o $@ $< $(LDFLAGS)

all: binary_search_runtimes

main.o: main.cpp utils.h measure_time.h custom_allocator.h binary_search_tree.h

clean:
	rm -f  *.o main

binary_search_runtimes: binary_search
	for number in $(WORKING_SET) ; do \
		./binary_search --working-set=$$number --prefetching-off --stride=0 --searches=$(NUMBER_OF_SEARCHES) ; \
	done
	for number in $(WORKING_SET) ; do \
		./binary_search --working-set=$$number --prefetching-on --stride=0 --searches=$(NUMBER_OF_SEARCHES); \
	done
	for number in $(WORKING_SET) ; do \
               ./binary_search --working-set=$$number --prefetching-off --stride=1 --searches=$(NUMBER_OF_SEARCHES) ; \
        done
	for number in $(WORKING_SET) ; do \
                ./binary_search --working-set=$$number --prefetching-on --stride=1 --searches=$(NUMBER_OF_SEARCHES); \
        done


binary_search_cache_misses: binary_search
	for number in $(WORKING_SET) ; do \
		$(PERF) ./binary_search --working-set=$$number --prefetching-off --stride=0 --searches=$(NUMBER_OF_SEARCHES) ; \
	done
	for number in $(WORKING_SET) ; do \
		$(PERF) ./binary_search --working-set=$$number --prefetching-on --stride=0 --searches=$(NUMBER_OF_SEARCHES); \
	done
	for number in $(WORKING_SET) ; do \
                $(PERF) ./binary_search --working-set=$$number --prefetching-off --stride=1 --searches=$(NUMBER_OF_SEARCHES) ; \
        done
	for number in $(WORKING_SET) ; do \
                $(PERF) ./binary_search --working-set=$$number --prefetching-on --stride=1 --searches=$(NUMBER_OF_SEARCHES); \
        done



profile:
	perf record --call-graph dwarf ./binary_search

report:
	perf report
