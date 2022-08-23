CC="clang++ -std=c++17 -g -O3 -mavx2 -fopenmp -DLIKWID_PERFMON -llikwid"

#$CC arithmetic_intensity.cpp -o arithmetic_intensity
#$CC read_vs_write.cpp -o read_vs_write
$CC memory_throughput.cpp -o memory_throughput
#$CC sequential_test.cpp -o sequential_test
