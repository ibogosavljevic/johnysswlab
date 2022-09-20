CC="clang++ -std=c++17 -g -O3 -mavx2 -fopenmp -DLIKWID_PERFMON -llikwid"

#$CC arithmetic_intensity.cpp -o arithmetic_intensity
#$CC read_vs_write.cpp -o read_vs_write
$CC numa_test.cpp -o numa_test
$CC binary_search_multithreading.cpp -o binary_search_multithreading
$CC memory_throughput.cpp -o memory_throughput
$CC memory_throughput_independent.cpp -o memory_throughput_independent
#$CC sequential_test.cpp -o sequential_test
