
CC=clang
OPT?=3
DEPS= 
LDFLAGS+=-lstdc++ -lm -fopenmp -llikwid
CFLAGS+=-I. -I.. -std=c++14 -O$(OPT) -pthread -g -Werror $(RPATH) -ffast-math


%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS)
	$(CC) -o $@ $< $(LDFLAGS)

all: filter_vector list_memory_layout vector_list

filter_vector.o: filter_vector.cpp filter_vector.h
list_memory_layout.o: list_memory_layout.cpp
vector_list.o: vector_list.cpp vector_list.h

format: filter_vector.cpp filter_vector.h list_memory_layout.cpp vector_list.cpp vector_list.h
	find . -name "*.cpp" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i
	find . -name "*.h" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i

run: 

clean:
	rm -f  *.o main

