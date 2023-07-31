clang++ -O3 -mavx2 -fopenmp -DLIKWID_PERFMON -I../common/utils.h -L/usr/lib/llvm-14/lib -g multithreading.cpp -o multithreading -lomp -llikwid
