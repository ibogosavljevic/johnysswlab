g++ -g -O3 -DLIKWID_PERFMON -fopenmp-simd -ffast-math -mavx2 image.cpp sum_column.cpp -o image -llikwid
g++ -g -O3 -DLIKWID_PERFMON -fopenmp-simd -ffast-math -mavx2 test.cpp square.cpp -o test
g++ -g -O3 -DLIKWID_PERFMON -fopenmp-simd -ffast-math -mavx2 complicated.cpp complicated-func.cpp -o complicated