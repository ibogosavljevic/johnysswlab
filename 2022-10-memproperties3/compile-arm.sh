CC="clang++ -std=c++17 -g -O3 -mcpu=cortex-a72 -mfpu=neon -fopenmp -DLIKWID_PERFMON -llikwid"

$CC linear_search_multithreading.cpp -o linear_search_multithreading
$CC binary_search_multithreading.cpp -o binary_search_multithreading
$CC memory_throughput.cpp -o memory_throughput
$CC memory_throughput_independent.cpp -o memory_throughput_independent
