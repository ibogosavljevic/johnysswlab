CC="clang++ -std=c++17 -g -O3 -mavx2 -fopenmp-simd -DLIKWID_PERFMON -llikwid"

$CC memory_latency.cpp -o memory_latency
$CC cache_line.cpp -o cache_line
