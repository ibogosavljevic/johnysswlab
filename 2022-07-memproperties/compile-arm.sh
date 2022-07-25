CC="clang++ -g -O3 -mcpu=cortex-a72 -mfpu=neon -fopenmp-simd -DLIKWID_PERFMON -llikwid"

$CC read_vs_write.cpp -o read_vs_write
$CC memory_throughput.cpp -o memory_throughput
$CC cache_line.cpp -o cache_line
$CC sequential_test.cpp -o sequential_test
