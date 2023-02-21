g++ -g -O3 -march=native -std=c++17 -DLIKWID_PERFMON sorting.cpp -o sorting -llikwid
g++ -g -O3 -march=native -std=c++17 -DLIKWID_PERFMON loop_fusion.cpp -o loop_fusion -llikwid
g++ -g -O3 -march=native -std=c++17 -I../ -DLIKWID_PERFMON ../2020-08-polymorphism/polymorphism.cpp -o polymorphism -llikwid
