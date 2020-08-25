
CC=g++
OPT?=3
DEPS= 
LDFLAGS+=-lpapi -lstdc++ -lm
FUNC_OPT?=0
CFLAGS+=-I. -I.. -std=c++17 -O$(OPT) -pthread -g -Werror $(RPATH) -DFUNC_OPT=$(FUNC_OPT) -DHAS_PAPI


WORKING_SET=10000 100000 1000000 10000000 100000000
NUMBER_OF_SEARCHES=10000000
PERF=perf stat -B -e cache-references,cache-misses,cache-misses:u,cycles,instructions,branches,branch-misses,faults,migrations 

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS)
	$(CC) -o $@ $< $(LDFLAGS)

all: polymorphism

polymorphism.o: polymorphism.cpp polymorphic_vector.h objects.h measure_time.h

format: polymorphism.cpp polymorphic_vector.h objects.h measure_time.h
	find . -name "*.cpp" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i
	find . -name "*.h" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i

clean:
	rm -f  *.o main

