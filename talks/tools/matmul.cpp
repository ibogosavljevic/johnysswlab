#include <likwid.h>
#include <array>
#include <cstdio>
#include <iostream>
#include <random>
#include "matrix.h"

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

template <typename T>
void fill(matrix<T>& m, T value) {
    for (int i = 0; i < m.rows(); i++) {
        for (int j = 0; j < m.cols(); j++) {
            m(i, j) = 1;
        }
    }
}

static constexpr std::array<int, 1> array_size = {1200};

int main(int argc, char** argv) {

    for (int i = 0; i < array_size.size(); i++) {
        int n = array_size[i];

        std::cout << "Matrix size = " << n << std::endl;

        matrix<double> out1(n, n), out2(n, n), in1(n, n), in2(n, n);

        fill_random(in1);
        fill_random(in2);

#if defined(MUL1)
        std::cout << "Mul1";
        matrix<double>::multiply_mul1(out1, in1, in2);
#else
        std::cout << "Mul2";
        matrix<double>::multiply_mul2(out2, in1, in2);
#endif
    }
    return 0;
}
