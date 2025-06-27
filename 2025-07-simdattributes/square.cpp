#include <immintrin.h>

//extern "C" double square(double x) {
//    return x*x;
//}

extern "C" __m256d _ZGVdN4v_square(__m256d x) {
    return _mm256_mul_pd(x,x);
}
