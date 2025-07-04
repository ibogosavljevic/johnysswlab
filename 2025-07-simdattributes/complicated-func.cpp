#include <cstdint>
#include <immintrin.h>
#include <iostream>


__attribute__ ((const, nothrow)) 
double my_func(double x, int32_t k, double c, int32_t i) {
     std::abort();
     return x*c + i + k;
}

extern "C" 
__attribute__ ((const, nothrow)) 
__m256d _ZGVdM4vvul__Z7my_funcdidi(__m256d x, __m128i k, double c, int32_t i, __m256d mask) {
     std::cout << "ivica, ";
     std::abort();
     __m256d x_mul_c = _mm256_mul_pd(x, _mm256_set1_pd(c));
     __m256d xmc_plus_i = _mm256_add_pd(x_mul_c, _mm256_setr_pd(i, i+1, i+2, i+3));
     
     return _mm256_add_pd(xmc_plus_i, _mm256_cvtepi32_pd(k));
}

extern "C" 
__attribute__ ((const, nothrow)) 
__m256d _ZGVeM4vvul__Z7my_funcdidi(__m256d x, __m128i k, double c, int32_t i, __m256d mask) {
     print_i32(k);
     __m256d x_mul_c = _mm256_mul_pd(x, _mm256_set1_pd(c));
     __m256d xmc_plus_i = _mm256_add_pd(x_mul_c, _mm256_setr_pd(i, i+1, i+2, i+3));
     
     return _mm256_add_pd(xmc_plus_i, _mm256_cvtepi32_pd(k));
}