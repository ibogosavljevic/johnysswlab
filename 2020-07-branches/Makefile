
CC=g++
OPT?=3
DEPS= 
#LDFLAGS=-lpapi
FUNC_OPT?=0
CFLAGS=-I. -std=c++11 -O$(OPT) -pthread -g -Werror $(RPATH) -DFUNC_OPT=$(FUNC_OPT)

SHELL := /bin/bash

WORKING_SET=10000 100000 1000000 10000000 100000000
NUMBER_OF_SEARCHES=10000000
TYPE=regular cmov arithmetic
PERF=perf stat -B -e cache-references,cache-misses,cache-misses:u,cycles,instructions,branches,branch-misses,faults,migrations 

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS)
	$(CC) -o $@ $< $(LDFLAGS)

all: binary_search_runtimes

binary_search.o: binary_search.cpp utils.h measure_time.h

counting.o: counting.cpp utils.h measure_time.h argparse.h

clean:
	rm -f  *.o binary_search counting

binary_search_runtimes: binary_search
	for number in $(WORKING_SET) ; do \
    for type in $(TYPE) ; do \
		./binary_search --test=$$type --length=$$number --prefetching=0 --searches=$(NUMBER_OF_SEARCHES) ; \
    done \
	done
	for number in $(WORKING_SET) ; do \
    for type in $(TYPE) ; do \
		./binary_search --test=$$type --length=$$number --prefetching=1 --searches=$(NUMBER_OF_SEARCHES); \
    done \
	done

binary_search_cache_misses: binary_search
	for number in $(WORKING_SET) ; do \
    for type in $(TYPE) ; do \
		$(PERF) ./binary_search --test=$$type --length=$$number --prefetching=0 --searches=$(NUMBER_OF_SEARCHES) ; \
	done \
    done
	for number in $(WORKING_SET) ; do \
    for type in $(TYPE) ; do \
		$(PERF) ./binary_search --test=$$type --length=$$number --prefetching=1 --searches=$(NUMBER_OF_SEARCHES); \
	done \
    done


profile:
	perf record --call-graph dwarf ./binary_search

report:
	perf report
