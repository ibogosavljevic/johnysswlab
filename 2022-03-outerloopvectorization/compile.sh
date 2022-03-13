clang++ -g -O3 -DLIKWID_PERFMON -mavx2 -fopenmp main.cpp  -llikwid
likwid-perfctr -m -g TMA -C S0 ./a.out
