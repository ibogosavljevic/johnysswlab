#include <immintrin.h>

double square(double x) {
    std::abort();
    return x*x;
}

extern "C" __m256d _ZGVdN4v__Z6squared(__m256d x) {
    return _mm256_mul_pd(x,x);
}

extern "C" __m256d _ZGVdM4v__Z6squared(__m256d x, __m256d mask) {
    return _mm256_mul_pd(x,x);
}
