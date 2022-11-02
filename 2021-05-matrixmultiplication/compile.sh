clang++ -march=core-avx2 -g -O3 -ffast-math main.cpp -o main -DLIKWID_PERFMON -llikwid
clang++ -march=core-avx2 -g -O3 -fno-slp-vectorize -fno-vectorize  -ffast-math main.cpp -o main-novec -DLIKWID_PERFMON -llikwid
