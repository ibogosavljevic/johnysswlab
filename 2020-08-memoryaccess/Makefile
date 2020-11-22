
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

all: main

main.o: main.cpp utils.h measure_time.h custom_allocator.h binary_search_tree.h

main: main.o

clean:
	rm -f  *.o main

profile:
	perf record --call-graph dwarf ./main

report:
	perf report
