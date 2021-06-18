#!/bin/sh
clang -O3 -ffast-math likwid-example.cpp -o likwid-example -lm -llikwid -lstdc++
