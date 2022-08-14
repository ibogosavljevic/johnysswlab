CC="clang++ -std=c++17 -g -O3 -mcpu=cortex-a72 -mfpu=neon -fopenmp -DLIKWID_PERFMON -llikwid"

$CC vectorization.cpp -o vectorization
$CC cache_conflicts.cpp -o cache_conflicts
$CC memory_throughput.cpp -o memory_throughput
$CC prefetching_streams.cpp -o prefetching_streams
$CC cache_line.cpp -o cache_line
$CC memory_latency.cpp -o memory_latency
