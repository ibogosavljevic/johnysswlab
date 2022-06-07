clang++ -g -O3  -DLIKWID_PERFMON  binary_tree.cpp -o binary_tree -llikwid  -ltcmalloc_minimal
clang++ -O3 -fno-math-errno -march=native -DLIKWID_PERFMON -g vector.cpp -o vector -llikwid
