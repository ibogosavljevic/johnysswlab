
#include "measure_time.h"
#include "utils.h"

#include <cstring>
#include <emmintrin.h>
#include <immintrin.h>


__attribute__ ((__target__ ("default")))
void add_default(float* __restrict__ a, float* __restrict__ b, float* __restrict__ res, int len) {
    float* __restrict__ aa = (float*) __builtin_assume_aligned(a, 128);
    float* __restrict__ ba = (float*) __builtin_assume_aligned(b, 128);
    float* __restrict__ resa = (float*) __builtin_assume_aligned(res, 128);

    for (int i = 0; i < len; i++) {
        resa[i] = aa[i] + ba[i];
    }
}


__attribute__ ((__target__ ("sse2")))
void add_sse_manual(float* __restrict__ a, float* __restrict__ b, float* __restrict__ res, int len) {
    __m128 aa, bb, rr;
    int n = len / 4;
    for (int i = 0; i < n; i += 4) {
        aa = _mm_load_ps(a + i);
        bb = _mm_load_ps(b + i);
        rr = _mm_add_ps(aa, bb);
        _mm_store_ps(res + i, rr);
    }
}


__attribute__ ((__target__ ("avx")))
void add_avx_manual(float* __restrict__ a, float* __restrict__ b, float* __restrict__ res, int len) {
    __m256 aa, bb, rr;
    int n = len / 8;
    for (int i = 0; i < n; i += 8) {
        aa = _mm256_load_ps(a + i);
        bb = _mm256_load_ps(b + i);
        rr = _mm256_add_ps(aa, bb);
        _mm256_store_ps(res + i, rr);
    }
}

__attribute__ ((__target__ ("default")))
void add2(float* __restrict__ a, float* __restrict__ b, float* __restrict__ res, int len) {
    add_default(a, b, res, len);
}


__attribute__ ((__target__ ("sse2")))
void add2(float* __restrict__ a, float* __restrict__ b, float* __restrict__ res, int len) {
    add_sse_manual(a, b, res, len);
}


__attribute__ ((__target__ ("avx")))
void add2(float* __restrict__ a, float* __restrict__ b, float* __restrict__ res, int len) {
    add_avx_manual(a, b, res, len);
}


typedef void add_t(float* __restrict__, float* __restrict__, float* __restrict__, int);

extern "C" {
    static add_t* add_dispatch() {
        __builtin_cpu_init ();
        if (__builtin_cpu_supports ("avx")) {
            return add_avx_manual;
        } else if (__builtin_cpu_supports ("sse2")) {
            return add_sse_manual;
        } else {
            return add_default;
        }
    }
}

__attribute__ ((ifunc ("add_dispatch")))
void add(float* __restrict__  a, float* __restrict__  b, float* __restrict__  res, int len);

constexpr int arr_len = 1024*1024*100;
float arr1[arr_len] __attribute__((aligned(128)));
float arr2[arr_len] __attribute__((aligned(128)));
float res[arr_len] __attribute__((aligned(128)));

int main(int argc, char** argv) {
    std::vector<float> array1 = create_random_array<float>(arr_len, 0.0, arr_len);
    std::vector<float> array2 = create_random_array<float>(arr_len, 0.0, arr_len);
    
    std::memcpy(arr1, &array1[0], sizeof(float) * arr_len);
    std::memcpy(arr2, &array2[1], sizeof(float) * arr_len);

    for (int i = 0; i < 100; i++) {
        measure_time m("manual dispatch");
        add(arr1, arr2, res, arr_len);
    }

    for (int i = 0; i < 100; i++) {
        measure_time m("automatic dispatch");
        add2(arr1, arr2, res, arr_len);
    }

    measure_time_database<std::chrono::milliseconds>::get_instance()->dump_database();

    return 0;
}
