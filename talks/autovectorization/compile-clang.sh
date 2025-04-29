clang++ -g -Rpass-analysis=loop-vectorize -Rpass=loop-vectorize -Rpass-missed=loop-vectorize -O3 -DLIKWID_PERFMON -mavx2 main.cpp -o main-clang -llikwid
