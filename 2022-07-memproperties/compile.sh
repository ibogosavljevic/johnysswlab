clang++ -O3 -mavx -fopenmp-simd -DLIKWID_PERFMON cache_line.cpp -o cache_line -llikwid
clang++ -O3 -mavx -fopenmp-simd -DLIKWID_PERFMON arithmetic_intensity.cpp -o arithmetic_intensity -llikwid
