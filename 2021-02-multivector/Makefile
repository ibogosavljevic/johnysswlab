
CC=clang
OPT?=3
DEPS= 
LDFLAGS+=-lstdc++ -lm -fopenmp
CFLAGS+=-I. -I.. -std=c++14 -O$(OPT) -pthread -g -Werror $(RPATH) -fopenmp -ffast-math


%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS)
	$(CC) -o $@ $< $(LDFLAGS)

all: main

main.o: main.cpp multivector.h

format: main.cpp multivector.h
	find . -name "*.cpp" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i
	find . -name "*.h" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i

clean:
	rm -f  *.o simd

