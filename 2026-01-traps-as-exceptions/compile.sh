g++ -fno-math-errno -O3 -DLIKWID_PERFMON -g -mavx2 main.cpp -o main-gcc -llikwid -lm
clang++ -fno-math-errno -O3 -DLIKWID_PERFMON -g -mavx2 main.cpp -o main-clang -llikwid -lm
