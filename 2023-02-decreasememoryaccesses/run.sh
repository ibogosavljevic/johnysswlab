likwid-perfctr -g TMA -m -C 0 ./loop_fusion
likwid-perfctr -g TMA -m -C 0 ./sorting
./polymorphism -a pointer -s 0
./polymorphism -a visitor -s 0
./polymorphism -a pointer -s 1
./polymorphism -a visitor -s 1
