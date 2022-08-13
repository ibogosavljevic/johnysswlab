

#include <iostream>
#include <string>
#include <random>
#include "../2021-05-matrixmultiplication/matrix.h"
#include "likwid.h"


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


int main(int argc, char **argv) {

    int start_size = 768;
    int end_size = 1280;

    LIKWID_MARKER_INIT;

    for (int n = start_size; n <= end_size; n += 16) {
        matrix<double> in1(n, n);
        matrix<double> in2(n, n);
        matrix<double> out1(n, n);
        matrix<double> out2(n, n);

        fill_random(in1);
        fill_random(in2);

        std::cout << "Running size " << n << std::endl;

        std::string naive_name = "naive_" + std::to_string(n);
        std::string interchanged_name = "interchanged_" + std::to_string(n);

        /*LIKWID_MARKER_START(naive_name.c_str());
        matrix<double>::multiply_interchanged(out1, in1, in2);
        LIKWID_MARKER_STOP(naive_name.c_str());*/

        LIKWID_MARKER_START(interchanged_name.c_str());
        matrix<double>::multiply_naive(out2, in1, in2);
        LIKWID_MARKER_STOP(interchanged_name.c_str());

        if (out1 != out2) {
            std::cout << "Not same\n";
        }
    }

    LIKWID_MARKER_CLOSE;

    return 0;
}