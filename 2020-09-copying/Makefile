
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

%.tidy: %.cpp
	clang-tidy -header-filter=.* -checks=portability-*,performance-*,readability-* $< -- $(CFLAGS)
	touch $@

all: main main_no_move_ctor main_no_ctor_init
tidy: main.tidy

main.o: bit_field.h main.cpp
main_no_move_ctor.o: bit_field.h main.cpp
	$(CC) -c -o $@ main.cpp $(CFLAGS) -DNO_MOVE_CTOR
main_no_ctor_init.o: bit_field.h main.cpp
	$(CC) -c -o $@ main.cpp $(CFLAGS) -DNO_CTOR_INIT -DNO_MOVE_CTR


format: main.cpp bit_field.h
	find . -name "*.cpp" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i
	find . -name "*.h" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i

clean:
	rm -f  *.o *.tidy main main_no_*

