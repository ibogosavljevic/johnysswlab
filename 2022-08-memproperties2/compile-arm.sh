CC="clang++ -std=c++17 -g -O3 -mcpu=cortex-a72 -mfpu=neon -fopenmp-simd -DLIKWID_PERFMON -llikwid"

$CC cache_line.cpp -o cache_line
$CC memory_latency.cpp -o memory_latency
