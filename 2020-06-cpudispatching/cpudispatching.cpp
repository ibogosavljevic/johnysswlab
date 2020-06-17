
#include "measure_time.h"
#include "utils.h"


#define ADD(name,arch) \
__attribute__ ((__target__ (arch)))  \
float add_##name(float* __restrict__ a, float* __restrict__ b, float* __restrict__ res, int len)  \
 { \
    std::cout << "Architecture: " << arch << "\n"; \
    for (int i = 0; i < len; i++) { \
        res[i] = a[i] + b[i]; \
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
        return add_sse4;
    }
}

__attribute__ ((ifunc ("add_dispatch")))
float add(float* __restrict__  a, float* __restrict__  b, float* __restrict__  res, int len);


int main(int argc, char** argv) {
    constexpr int arr_len = 1024*1024*100;
    std::vector<float> array1 = create_random_array<float>(arr_len, 0.0, arr_len);
    std::vector<float> array2 = create_random_array<float>(arr_len, 0.0, arr_len);
    std::vector<float> res(arr_len);

    for (int i = 0; i < 50; i++) {
        measure_time m("test");
        add(&array1[0], &array2[0], &res[0], arr_len);
    }

    measure_time_database<std::chrono::milliseconds>::get_instance()->dump_database();

    return 0;
}