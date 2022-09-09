CC="clang++ -std=c++17 -g -O3 -mcpu=cortex-a72 -mfpu=neon -fopenmp-simd -DLIKWID_PERFMON -llikwid"

$CC memory_throughput.cpp -o memory_throughput
$CC memory_throughput_independent.cpp -o memory_throughput_independent
