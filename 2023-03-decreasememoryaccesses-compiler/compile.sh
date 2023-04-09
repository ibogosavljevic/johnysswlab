clang++ -O3 -g -mavx2 -DLIKWID_PERFMON aliasing.cpp -o aliasing -llikwid -fsave-optimization-record
g++ -O3 -g -mavx2 -DLIKWID_PERFMON aliasing.cpp -o aliasing-gcc -llikwid
