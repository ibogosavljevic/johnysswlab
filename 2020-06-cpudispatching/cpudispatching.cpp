
#include "measure_time.h"
#include "utils.h"

#include <cstring>


#define ADD(name,arch) \
__attribute__ ((__target__ (arch)))  \
float add_##name(float* __restrict__ a, float* __restrict__ b, float* __restrict__ res, int len)  \
 { \
    float* __restrict__ aa = (float*) __builtin_assume_aligned(a, 128); \
    float* __restrict__ ba = (float*) __builtin_assume_aligned(b, 128); \
    float* __restrict__ resa = (float*) __builtin_assume_aligned(res, 128); \
    std::cout << "Architecture: " << arch << "\n"; \
    for (int i = 0; i < len; i++) { \
        resa[i] = aa[i] + ba[i]; \
    } \
    return 0.0; \
}

ADD(default, "default")
ADD(sse4, "sse4")
ADD(avx, "avx")
ADD(avx2, "avx2")


typedef float add_t(float* __restrict__, float* __restrict__, float* __restrict__, int);


extern "C" {
    static add_t* add_dispatch() {
        return add_avx;
    }
}

__attribute__ ((ifunc ("add_dispatch")))
float add(float* __restrict__  a, float* __restrict__  b, float* __restrict__  res, int len);

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
        measure_time m("test");
        add(arr1, arr2, res, arr_len);
    }

    measure_time_database<std::chrono::milliseconds>::get_instance()->dump_database();

    return 0;
}