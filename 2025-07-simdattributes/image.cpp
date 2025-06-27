#include <vector>
#include <random>
#include "likwid.h"
#include <iostream>

static void clobber() {
    asm volatile("" : : : "memory");
}


std::vector<double> generateRandomDoubles(std::size_t size) {
    std::vector<double> result;
    result.reserve(size);  // Reserve memory for efficiency

    std::random_device rd;  // Seed generator
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_real_distribution<> dist(-100.0, 100.0);

    for (std::size_t i = 0; i < size; ++i) {
        result.push_back(dist(gen));
    }

    return result;
}

extern "C" {

#pragma omp declare simd uniform(img_ptr, width, height) linear(column) notinbranch
__attribute__ ((pure, nothrow)) 
double sum_row(double const * const img_ptr, size_t column, size_t width, size_t height);

}

int main() {
    LIKWID_MARKER_INIT;

    static constexpr size_t WIDTH = 3841;
    static constexpr size_t HEIGHT = 2161;

    std::vector<double> img = generateRandomDoubles(WIDTH * HEIGHT);
    std::vector<double> sum_columns0(WIDTH);
    std::vector<double> sum_columns1(WIDTH);

    double const * const img_ptr { img.data() };

    LIKWID_MARKER_START("scalar_func");

    for (size_t i = 0; i < WIDTH; i++) {
        sum_columns0[i] = sum_row(img_ptr, i, WIDTH, HEIGHT);
    }

    clobber();

    LIKWID_MARKER_STOP("scalar_func");

    LIKWID_MARKER_START("vector_func");

    #pragma omp simd
    for (size_t i = 0; i < WIDTH; i++) {
        sum_columns1[i] = sum_row(img_ptr, i, WIDTH, HEIGHT);
    }

    clobber();

    bool same = true;
    for (size_t i = 0; i < WIDTH; i++) {
        if (sum_columns0[i] != sum_columns1[i]) {
            same = false;
            break;
        }
    }

    std::cout << "Results same = " << (same ? "TRUE" : "FALSE") << "\n";

    LIKWID_MARKER_STOP("vector_func");

    LIKWID_MARKER_CLOSE;
}

