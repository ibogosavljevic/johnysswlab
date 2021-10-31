clang++ -O3 main.cpp -o main -llikwid
sudo likwid-perfctr -m -g MEM -C 0 ./main
