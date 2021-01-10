
CC=g++
OPT?=3
DEPS= 
LDFLAGS+=-lstdc++ -lm -fopenmp
CFLAGS+=-I. -I.. -std=c++11 -O$(OPT) -pthread -g -Werror $(RPATH) -fopenmp -march=native


%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS)
	$(CC) -o $@ $< $(LDFLAGS)

all: sort

sort.o: sort.cpp utils.h measure_time.h

format: sort.cpp utils.h measure_time.h
	find . -name "*.cpp" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i
	find . -name "*.h" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i

clean:
	rm -f  *.o parallel prefix_sums

