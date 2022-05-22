clang++ -g -O3  -DLIKWID_PERFMON  binary_tree.cpp  -llikwid  -ltcmalloc_minimal
clang++ -O3 -fno-math-errno -march=native -DLIKWID_PERFMON -g vector.cpp -llikwid
