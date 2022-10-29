clang++ -O3 -mavx2 -ffast-math -DLIKWID_PERFMON loop-sectioning.cpp -o loop-sectioning -llikwid -Rpass=loop-vectorize -Rpass-missed=loop-vectorize -Rpass-analysis=loop-vectorize
