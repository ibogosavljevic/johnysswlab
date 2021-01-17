
CC=clang
OPT?=3
DEPS= 
LDFLAGS+=-lpapi -lstdc++ -lm -ltbb -fopenmp
CFLAGS+=-I. -I.. -std=c++11 -O$(OPT) -pthread -g -Werror $(RPATH) -fopenmp


%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS)
	$(CC) -o $@ $< $(LDFLAGS)

all: binarysearch

binarysearch.o: binarysearch.cpp utils.h measure_time.h

format: binarysearch.cpp utils.h measure_time.h
	find . -name "*.cpp" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i
	find . -name "*.h" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i

clean:
	rm -f  *.o binarysearch

