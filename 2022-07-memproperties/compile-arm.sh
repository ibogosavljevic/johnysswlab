clang++ -g -O3 -mcpu=cortex-a72 -mfpu=neon -fopenmp-simd -DLIKWID_PERFMON cache_line.cpp -o cache_line -llikwid
clang++ -g -O3 -mcpu=cortex-a72 -mfpu=neon -fopenmp-simd -DLIKWID_PERFMON arithmetic_intensity.cpp -o arithmetic_intensity -llikwid
