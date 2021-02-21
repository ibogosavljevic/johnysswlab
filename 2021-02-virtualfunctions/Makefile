
CC=clang
OPT?=3
DEPS= 
LDFLAGS+=-lstdc++ -lm -fopenmp
CFLAGS+=-I. -I.. -std=c++14 -O$(OPT) -pthread -g -Werror $(RPATH) -fopenmp -ffast-math -fsave-optimization-record -foptimization-record-file=myOptfile.yaml 


%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS)
	$(CC) -o $@ $< $(LDFLAGS)

all: main

main.o: main.cpp multivector.h objects.h polymorphic_vector.h

format: main.cpp multivector.h objects.h polymorphic_vector.h
	find . -name "*.cpp" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i
	find . -name "*.h" | xargs clang-format -style="{BasedOnStyle: Chromium, IndentWidth: 4}" -i

clean:
	rm -f  *.o main

