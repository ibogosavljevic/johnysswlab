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

static constexpr std::array<int, 3> array_size = {240, 1200, 1680};

int main(int argc, char** argv) {
    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;

    for (int i = 0; i < array_size.size(); i++) {
        int n = array_size[i];

        std::cout << "Matrix size = " << n << std::endl;

        matrix<double> out1(n, n), out2(n, n), in1(n, n), in2(n, n);

        fill_random(in1);
        fill_random(in2);

        {
            std::string name = "Naive_" + std::to_string(n);
            LIKWID_MARKER_START(name.c_str());
            matrix<double>::multiply_naive(out1, in1, in2);
            LIKWID_MARKER_STOP(name.c_str());
        }
        {
            std::string name = "Interchanged_" + std::to_string(n);
            LIKWID_MARKER_START(name.c_str());
            matrix<double>::multiply_interchanged(out2, in1, in2);
            LIKWID_MARKER_STOP(name.c_str());
            if (out1 != out2) {
                std::cout << "MATRICES NOT SAME!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }

        {
            std::string name = "Tiled_2_" + std::to_string(n);
            LIKWID_MARKER_START(name.c_str());
            matrix<double>::multiply_tiled<2>(out2, in1, in2);
            LIKWID_MARKER_STOP(name.c_str());

            if (out1 != out2) {
                std::cout << "MATRICES NOT SAME!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            std::string name = "Tiled_3_" + std::to_string(n);
            LIKWID_MARKER_START(name.c_str());
            matrix<double>::multiply_tiled<3>(out2, in1, in2);
            LIKWID_MARKER_STOP(name.c_str());

            if (out1 != out2) {
                std::cout << "MATRICES NOT SAME!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            std::string name = "Tiled_4_" + std::to_string(n);
            LIKWID_MARKER_START(name.c_str());
            matrix<double>::multiply_tiled<4>(out2, in1, in2);
            LIKWID_MARKER_STOP(name.c_str());

            if (out1 != out2) {
                std::cout << "MATRICES NOT SAME!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            std::string name = "Tiled_8_" + std::to_string(n);
            LIKWID_MARKER_START(name.c_str());
            matrix<double>::multiply_tiled<8>(out2, in1, in2);
            LIKWID_MARKER_STOP(name.c_str());

            if (out1 != out2) {
                std::cout << "MATRICES NOT SAME!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            std::string name = "Tiled_12_" + std::to_string(n);
            LIKWID_MARKER_START(name.c_str());
            matrix<double>::multiply_tiled<12>(out2, in1, in2);
            LIKWID_MARKER_STOP(name.c_str());

            if (out1 != out2) {
                std::cout << "MATRICES NOT SAME!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            std::string name = "Tiled_AVX_8_" + std::to_string(n);
            LIKWID_MARKER_START(name.c_str());
            matrix<double>::multiply_tiled_avx(out2, in1, in2, 8);
            LIKWID_MARKER_STOP(name.c_str());

            if (out1 != out2) {
                std::cout << "MATRICES NOT SAME!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            std::string name = "Tiled_AVX_12_" + std::to_string(n);
            LIKWID_MARKER_START(name.c_str());
            matrix<double>::multiply_tiled_avx(out2, in1, in2, 12);
            LIKWID_MARKER_STOP(name.c_str());

            if (out1 != out2) {
                std::cout << "MATRICES NOT SAME!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            std::string name = "Tiled_AVX_24_" + std::to_string(n);
            LIKWID_MARKER_START(name.c_str());
            matrix<double>::multiply_tiled_avx(out2, in1, in2, 24);
            LIKWID_MARKER_STOP(name.c_str());

            if (out1 != out2) {
                std::cout << "MATRICES NOT SAME!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            std::string name = "Tiled_AVX_48_" + std::to_string(n);
            LIKWID_MARKER_START(name.c_str());
            matrix<double>::multiply_tiled_avx(out2, in1, in2, 48);
            LIKWID_MARKER_STOP(name.c_str());

            if (out1 != out2) {
                std::cout << "MATRICES NOT SAME!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            std::string name = "Tiled_AVX_60_" + std::to_string(n);
            LIKWID_MARKER_START(name.c_str());
            matrix<double>::multiply_tiled_avx(out2, in1, in2, 60);
            LIKWID_MARKER_STOP(name.c_str());

            if (out1 != out2) {
                std::cout << "MATRICES NOT SAME!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
        {
            std::string name = "Tiled_AVX_80_" + std::to_string(n);
            LIKWID_MARKER_START(name.c_str());
            matrix<double>::multiply_tiled_avx(out2, in1, in2, 80);
            LIKWID_MARKER_STOP(name.c_str());

            if (out1 != out2) {
                std::cout << "MATRICES NOT SAME!!!\n";
            } else {
                std::cout << "Matrices same!!!\n";
            }
        }
    }

    LIKWID_MARKER_CLOSE;

    return 0;
}
