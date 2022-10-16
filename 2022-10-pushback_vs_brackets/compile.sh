clang++ -O3 -DLIKWID_PERFMON -g -ffast-math -mavx2 -fopenmp test.cpp -o test -llikwid -Rpass=loop-vectorize -Rpass-missed=loop-vectorize -Rpass-analysis=loop-vectorize
