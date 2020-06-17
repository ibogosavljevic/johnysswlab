
CC=g++
OPT?=3
CFLAGS=-I. -std=c++11 -O$(OPT) -pthread -ggdb3 -Werror $(RPATH)
DEPS = 

WORKING_SET=10000 100000 1000000 10000000 100000000
NUMBER_OF_SEARCHES=10000000
PERF=perf stat -B -e cache-references,cache-misses,cache-misses:u,cycles,instructions,branches,branch-misses,faults,migrations 

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS)
	$(CC) -o $@ $< $(CFLAGS)

cpudispatching.o: cpudispatching.cpp utils.h measure_time.h

clean:
	rm -f  *.o cpudispatching

