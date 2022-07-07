CC="clang++ -g -O3 -mcpu=cortex-a72 -mfpu=neon -fopenmp-simd -DLIKWID_PERFMON -llikwid"

$CC memory_throughput.cpp -o memory_throughput
$CC cache_line.cpp -o cache_line
$CC arithmetic_intensity.cpp -o arithmetic_intensity
