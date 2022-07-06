CC="clang++ -O3 -mavx2 -fopenmp-simd -DLIKWID_PERFMON -llikwid"

$CC memory_throughput.cpp -o memory_throughput
$CC cache_line.cpp -o cache_line
$CC arithmetic_intensity.cpp -o arithmetic_intensity
