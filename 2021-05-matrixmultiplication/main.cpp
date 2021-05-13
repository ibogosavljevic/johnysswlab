#include <array>
#include <cstdio>
#include <iostream>
#include <random>
#include "matrix.h"
#include "measure_time.h"

template <typename T>
void fill_random(matrix<T>& m) {
    std::uniform_real_distribution<T> unif{};
    std::default_random_engine re;

    for (int i = 0; i < m.rows(); i++) {
        for (int j = 0; j < m.cols(); j++) {
            m(i, j) = unif(re);
        }
    }
}

static constexpr std::array<int, 4> array_size = {24, 240, 1200, 1680};

int main(int argc, char** argv) {
    for (int i = 0; i < array_size.size(); i++) {
        int n = array_size[i];

        std::cout << "Matrix size = " << n << std::endl;

        matrix<double> out1(n, n), out2(n, n), in1(n, n), in2(n, n);

        fill_random(in1);
        fill_random(in2);

        {
            measure_time m("Naive");
            matrix<double>::multiply_naive(out1, in1, in2);
        }
        {
            measure_time m("Interchanged");
            matrix<double>::multiply_interchanged(out2, in1, in2);
            if (out1 != out2) {
                std::cout << "Matrices not same!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }

        {
            measure_time m("Tile size 2");
            matrix<double>::multiply_tiled<2>(out2, in1, in2);
            if (out1 != out2) {
                std::cout << "Matrices not same!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            measure_time m("Tile size 3");
            matrix<double>::multiply_tiled<3>(out2, in1, in2);
            if (out1 != out2) {
                std::cout << "Matrices not same!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            measure_time m("Tile size 4");
            matrix<double>::multiply_tiled<4>(out2, in1, in2);
            if (out1 != out2) {
                std::cout << "Matrices not same!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            measure_time m("Tile size 8");
            matrix<double>::multiply_tiled<8>(out2, in1, in2);
            if (out1 != out2) {
                std::cout << "Matrices not same!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            measure_time m("Tile size 12");
            matrix<double>::multiply_tiled<12>(out2, in1, in2);
            if (out1 != out2) {
                std::cout << "Matrices not same!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
    }

    return 0;
}
