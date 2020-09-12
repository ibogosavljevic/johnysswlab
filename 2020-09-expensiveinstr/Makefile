
CC=g++
OPT?=3
DEPS= 
LDFLAGS+=-lpapi -lstdc++ -lm
FUNC_OPT?=0
CFLAGS+=-I. -I.. -std=c++11 -O$(OPT) -pthread -g -Werror $(RPATH) -DFUNC_OPT=$(FUNC_OPT) -DHAS_PAPI


%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS)
	$(CC) -o $@ $< $(LDFLAGS)

all: hashing_test floating_test

hashing_test.o: hashing_test.cpp utils.h hash_map.h
floating_test.o: floating_test.cpp utils.h fix16.h fix16.hpp

format: hashing_test.cpp utils.h hash_map.h
	find . -name "*.cpp" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i
	find . -name "*.h" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i

clean:
	rm -f  *.o hashing_test floating_test

