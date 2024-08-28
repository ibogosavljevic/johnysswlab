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
result_t calculate_pi(double error) {
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

    } while((std::abs(sum_old - sum) > error));

    //std::cout << "1) i2 = " << i2 << "\n";

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

result_t calculate_pi_vector(double error) {
    __m256d sum = _mm256_set1_pd(0.0);

    __m256d i1 = _mm256_setr_pd(1.0, 5.0, 9.0, 13.0);
    __m256d i2 = _mm256_setr_pd(3.0, 7.0, 11.0, 15.0);
    const __m256d one = _mm256_set1_pd(1.0);
    const __m256d one_minus = _mm256_set1_pd(-1.0);
    const __m256d inc = _mm256_set1_pd(16.0);

    const __m256d clean_last_lane = _mm256_castsi256_pd(_mm256_setr_epi64x(-1, -1, -1, 0));

    double sum_old_v, sum_v;

    do {
        for(size_t i = 0; i < 7; i++) {
            __m256d x1 = _mm256_div_pd(one, i1);
            __m256d x2 = _mm256_div_pd(one_minus, i2);

            i1 = _mm256_add_pd(i1, inc);
            i2 = _mm256_add_pd(i2, inc);

            sum = _mm256_add_pd(sum, _mm256_add_pd(x1, x2));
        }

        __m256d sum_old;
        {
            __m256d x1 = _mm256_div_pd(one, i1);
            __m256d x2 = _mm256_div_pd(one_minus, i2);

            i1 = _mm256_add_pd(i1, inc);
            i2 = _mm256_add_pd(i2, inc);

            __m256d x1_no_last_lane = _mm256_and_pd(x1, clean_last_lane);
            __m256d x2_no_last_lane = _mm256_and_pd(x2, clean_last_lane);

            sum_old = _mm256_add_pd(sum, x1_no_last_lane);
            sum_old = _mm256_add_pd(sum, x2_no_last_lane);

            sum = _mm256_add_pd(sum, x1);
            sum = _mm256_add_pd(sum, x2);
        }

        sum_old_v = reduce(sum_old);
        sum_v = reduce(sum);

    } while ((std::abs(sum_old_v - sum_v) > error));

    double x[4];
    _mm256_store_pd(x, i2);

    return { sum_v, x[3] };
}

result_t calculate_pi2(double error) {
    double sum_old = 0.0;
    double sum = 0.0;
    double sum0 = 0.0, sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;

    double i1 = 1.0;
    double i2 = 3.0;

    do {
        double x1 = 1.0 / i1;
        double x2 = -1.0 / i2;

        i1 += 4.0;
        i2 += 4.0;

        sum0 += x1 + x2;

        x1 = 1.0 / i1;
        x2 = -1.0 / i2;

        i1 += 4.0;
        i2 += 4.0;

        sum1 += x1 + x2;

        x1 = 1.0 / i1;
        x2 = -1.0 / i2;

        i1 += 4.0;
        i2 += 4.0;

        sum2 += x1 + x2;

        x1 = 1.0 / i1;
        x2 = -1.0 / i2;

        i1 += 4.0;
        i2 += 4.0;

        double sum3_old = sum3;
        sum3 += x1 + x2;
        
        sum_old = sum0 + sum1 + sum2 + sum3_old;
        sum = sum0 + sum1 + sum2 + sum3;

    } while((std::abs(sum_old - sum) > error));

    return { sum, i2 };
}

int main(int argc, char** argv) {
    static constexpr double pi4 = M_PI_4;
    
    LIKWID_MARKER_INIT;

    LIKWID_MARKER_START("compiler_pi");
    result_t pi_compiler = calculate_pi(0.0);
    LIKWID_MARKER_STOP("compiler_pi");

    LIKWID_MARKER_START("vector_pi1");
    result_t pi_vector1 = calculate_pi_vector(1e-11);
    LIKWID_MARKER_STOP("vector_pi1");

    LIKWID_MARKER_START("vector_pi2");
    result_t pi_vector2 = calculate_pi_vector(1e-10);
    LIKWID_MARKER_STOP("vector_pi2");

    LIKWID_MARKER_START("vector_pi3");
    result_t pi_vector3 = calculate_pi_vector(1e-9);
    LIKWID_MARKER_STOP("vector_pi3");

    LIKWID_MARKER_START("vector_pi4");
    result_t pi_vector4 = calculate_pi_vector(1.3e-8);
    LIKWID_MARKER_STOP("vector_pi4");

    LIKWID_MARKER_START("compiler_pi2");
    result_t pi_compiler2 = calculate_pi2(0);
    LIKWID_MARKER_STOP("compiler_pi2");

    std::cout << std::setprecision (std::numeric_limits<double>::digits10 + 1);
    std::cout << "compiler diff " << std::abs(pi4 - pi_compiler.pi_4) << "\n";
    std::cout << "vector1 diff " << std::abs(pi4 - pi_vector1.pi_4) << "\n";
    std::cout << "vector2 diff " << std::abs(pi4 - pi_vector2.pi_4) << "\n";
    std::cout << "vector3 diff " << std::abs(pi4 - pi_vector3.pi_4) << "\n";
    std::cout << "vector4 diff " << std::abs(pi4 - pi_vector4.pi_4) << "\n";
    std::cout << "compiler2 diff " << std::abs(pi4 - pi_compiler2.pi_4) << "\n";

    std::cout << "compiler last member " << pi_compiler.last_member << "\n";
    std::cout << "vector1 last member " << pi_vector1.last_member << "\n";
    std::cout << "vector2 last member " << pi_vector2.last_member << "\n";
    std::cout << "vector3 last member " << pi_vector3.last_member << "\n";
    std::cout << "vector4 last member " << pi_vector4.last_member << "\n";
    std::cout << "compiler2 last member " << pi_compiler2.last_member << "\n";

    LIKWID_MARKER_CLOSE;

}