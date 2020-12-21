
CC=clang
OPT?=3
DEPS= 
LDFLAGS+=-lpapi -lstdc++ -lm -pthread
FUNC_OPT?=0
CFLAGS+=-I. -I.. -std=c++11 -O$(OPT) -pthread -g -Werror $(RPATH) -DFUNC_OPT=$(FUNC_OPT) -DHAS_PAPI

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS)
	$(CC) -o $@ $< $(LDFLAGS)

all: multithreading

multithreading.o: multithreading.cpp utils.h measure_time.h spinlock.h

format: multithreading.cpp utils.h measure_time.h spinlock.h
	find . -name "*.cpp" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i
	find . -name "*.h" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i

clean:
	rm -f  *.o multithreading

