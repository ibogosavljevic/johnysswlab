#include <iostream>
#include <immintrin.h>

void print_f64(__m256d d) {
    alignas(64) double v[4];
    
    _mm256_store_pd(v, d);
    for (size_t i = 0; i < 4; i++) {
        std::cout << v[i] << ", ";
    }
    std::cout << "\n";
}

extern "C" {

__attribute__ ((const, nothrow)) 
__m256d _ZGVdN4uluu_sum_column(double const * const img_ptr, size_t column, size_t width, size_t height) {
    __m256d s = _mm256_set1_pd(0.0);
    for (size_t j = 0; j < height; j++) {
        __m256d v = _mm256_loadu_pd(img_ptr + j * width + column);
        s = _mm256_add_pd(s, v);
    }
    return s;
}

__attribute__ ((const, nothrow)) 
double sum_column(double const * const img_ptr, size_t column, size_t width, size_t height) {
    double s = 0.0;
    for (size_t j = 0; j < height; j++) {
        s += img_ptr[j * width + column];
    }
    return s;
}

}