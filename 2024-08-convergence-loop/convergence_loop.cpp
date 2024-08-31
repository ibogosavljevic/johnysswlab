#include <iostream>
#include <iomanip>
#include <immintrin.h>
#include <cmath>
#include "likwid.h"

struct result_t {
    double pi_4;
    double last_member;
};

// Calculates pi using Leibnitz formula
result_t calculate_pi(double convergence_diff) {
    // Leibnitz formula 1 - 1/3 + 1/5 - 1/7 ...
    double sum_old = 0.0;
    double sum = 0.0;

    double i1 = 1.0;
    double i2 = 3.0;

    do {
        double x1 = 1.0 / i1;
        double x2 = -1.0 / i2;

        i1 += 4.0;
        i2 += 4.0;

        sum_old = sum;
        sum += x1 + x2;

    } while((std::abs(sum_old - sum) > convergence_diff));

    return { sum, i2 };
}


double reduce(__m256d v) {
    double sum = 0.0;
    alignas(32) double tmp[4];

    _mm256_store_pd(tmp, v);
    for (size_t i = 0; i < 4; i++) {
        sum += tmp[i];
    }

    return sum;
}

result_t calculate_pi_vector1(double convergence_diff) {
    __m256d sum = _mm256_set1_pd(0.0);

    __m256d i1 = _mm256_setr_pd(1.0, 5.0, 9.0, 13.0);
    __m256d i2 = _mm256_setr_pd(3.0, 7.0, 11.0, 15.0);
    const __m256d one = _mm256_set1_pd(1.0);
    const __m256d one_minus = _mm256_set1_pd(-1.0);
    const __m256d inc = _mm256_set1_pd(16.0);

    const __m256d clean_last_lane = _mm256_castsi256_pd(_mm256_setr_epi64x(-1, -1, -1, 0));

    alignas(32) double tmp[4];

    do {
        __m256d x1 = _mm256_div_pd(one, i1);
        __m256d x2 = _mm256_div_pd(one_minus, i2);

        i1 = _mm256_add_pd(i1, inc);
        i2 = _mm256_add_pd(i2, inc);

        __m256d x = _mm256_add_pd(x1, x2);
        sum = _mm256_add_pd(sum, x);

        _mm256_store_pd(tmp, x);

    } while (tmp[3] > convergence_diff);

    _mm256_store_pd(tmp, i2);

    return { reduce(sum), tmp[3] };
}


result_t calculate_pi_vector2(double convergence_diff) {
    __m256d sum = _mm256_set1_pd(0.0);

    __m256d i1 = _mm256_setr_pd(1.0, 5.0, 9.0, 13.0);
    __m256d i2 = _mm256_setr_pd(3.0, 7.0, 11.0, 15.0);
    const __m256d one = _mm256_set1_pd(1.0);
    const __m256d one_minus = _mm256_set1_pd(-1.0);
    const __m256d inc = _mm256_set1_pd(16.0);

    alignas(32) double tmp[4];

    for(;;) {
        __m256d little_sum = _mm256_set1_pd(0.0);

        for (size_t i = 0; i < 256; i++) {
            __m256d x1 = _mm256_div_pd(one, i1);
            __m256d x2 = _mm256_div_pd(one_minus, i2);

            i1 = _mm256_add_pd(i1, inc);
            i2 = _mm256_add_pd(i2, inc);

            __m256d x = _mm256_add_pd(x1, x2);
            little_sum = _mm256_add_pd(little_sum, x);

            _mm256_store_pd(tmp, x);

            if (tmp[3] <= convergence_diff) {
                sum = _mm256_add_pd(sum, little_sum);
                goto exit;
            }
        }

        sum = _mm256_add_pd(sum, little_sum);
    }

exit:
    _mm256_store_pd(tmp, i2);

    return { reduce(sum), tmp[3] };
}

int main(int argc, char** argv) {
    static constexpr double pi4 = M_PI_4;
    
    LIKWID_MARKER_INIT;

    LIKWID_MARKER_START("compiler_pi");
    result_t pi_compiler = calculate_pi(0.0);
    LIKWID_MARKER_STOP("compiler_pi");

    LIKWID_MARKER_START("vector_pi1");
    result_t pi_vector1 = calculate_pi_vector1(1e-18);
    LIKWID_MARKER_STOP("vector_pi1");

    LIKWID_MARKER_START("vector_pi2");
    result_t pi_vector2 = calculate_pi_vector1(1e-17);
    LIKWID_MARKER_STOP("vector_pi2");

    LIKWID_MARKER_START("vector_pi3");
    result_t pi_vector3 = calculate_pi_vector1(4e-17);
    LIKWID_MARKER_STOP("vector_pi3");

    LIKWID_MARKER_START("vector2_pi1");
    result_t pi_vector21 = calculate_pi_vector2(4e-17);
    LIKWID_MARKER_STOP("vector2_pi1");

    std::cout << std::setprecision (std::numeric_limits<double>::digits10 + 1);
    std::cout << "compiler diff " << std::abs(pi4 - pi_compiler.pi_4) << "\n";
    std::cout << "vector1 diff " << std::abs(pi4 - pi_vector1.pi_4) << "\n";
    std::cout << "vector2 diff " << std::abs(pi4 - pi_vector2.pi_4) << "\n";
    std::cout << "vector3 diff " << std::abs(pi4 - pi_vector3.pi_4) << "\n";
    std::cout << "vector2x diff " << std::abs(pi4 - pi_vector21.pi_4) << "\n";

    std::cout << "compiler last member " << pi_compiler.last_member << "\n";
    std::cout << "vector1 last member " << pi_vector1.last_member << "\n";
    std::cout << "vector2 last member " << pi_vector2.last_member << "\n";
    std::cout << "vector3 last member " << pi_vector3.last_member << "\n";
    std::cout << "vector2x last member " << pi_vector21.last_member << "\n";


    LIKWID_MARKER_CLOSE;

}