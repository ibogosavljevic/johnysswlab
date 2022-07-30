CC="clang++ -std=c++17 -g -O3 -mcpu=cortex-a72 -mfpu=neon -fopenmp-simd -DLIKWID_PERFMON -llikwid"

$CC -DARRAY_SIZE=8*1024*1024 cache_line.cpp -o cache_line
