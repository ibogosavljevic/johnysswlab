
CC=g++
OPT?=3
DEPS= 
LDFLAGS+=-lpapi -lstdc++ -lm
FUNC_OPT?=0
CFLAGS+=-I. -I.. -std=c++14 -O$(OPT) -pthread -g -Werror $(RPATH) -DFUNC_OPT=$(FUNC_OPT) -DHAS_PAPI


%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS)
	$(CC) -o $@ $< $(LDFLAGS)

all: access_pattern_test allocation_test

allocation_test.o: allocation_test.cpp

access_pattern_test.o: access_pattern_test.cpp utils.h  measure_time.h

format: allocation_test.cpp access_pattern_test.cpp utils.h  measure_time.h
	find . -name "*.cpp" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i
	find . -name "*.h" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i

run_allocation_test: allocation_test
	echo "No allocator"
	multitime -f rusage -n 5  ./allocation_test
	echo "Tcmalloc"
	LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc_minimal.so.4 multitime -f rusage -n 5  ./allocation_test
	echo "Jemalloc"
	LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2 multitime -f rusage -n 5 ./allocation_test

clean:
	rm -f  *.o access_pattern_test allocation_test

