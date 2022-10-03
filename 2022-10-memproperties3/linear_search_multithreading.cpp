#include <vector>
#include "likwid.h"
#include "omp.h"

#include <cassert>
#include <string>
#include <algorithm>
#include <numeric>
#include <iostream>

#ifdef __AVX2__
#include <immintrin.h>
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

static void clobber() {
  asm volatile("" : : : "memory");
}

struct binary_search_result_t {
    int index;
    long memory_accesses;
};

#undef __AVX2__

#ifdef __AVX2__

#define VECTOR_SIZE 8
using int_vec = __m256i;

int_vec load_vec(const int32_t* addr) {
    return _mm256_loadu_si256((const __m256i_u *) addr);
}

int_vec set_vec(int32_t val) {
    return _mm256_set1_epi32(val);
}

bool equal_any(int_vec val1, int_vec val2) {
    __m256i res = _mm256_cmpeq_epi32(val1, val2);
    return (_mm256_movemask_ps(_mm256_castsi256_ps(res)) != 0);
}


#elif defined(__ARM_NEON)

#define VECTOR_SIZE 4
using int_vec = int32x4_t;

int_vec load_vec(const int32_t* addr) {
    return vld1q_s32(addr);(addr);
}

int_vec set_vec(int32_t val) {
    return vmovq_n_s32(val);
}

bool equal_any(int_vec val1, int_vec val2) {
    uint32x4_t res = vceqq_s32(val1, val2);
    return vmaxvq_u32(res) != 0;
}

void store_vec(int* addr, int_vec value) {
    vst1q_s32(addr, value);
}

#else

#define VECTOR_SIZE 1
using int_vec = int32_t;

int_vec load_vec(const int32_t* addr) {
    return *addr;
}

int_vec set_vec(int32_t val) {
    return val;
}

bool equal_any(int_vec val1, int_vec val2) {
    return val1 == val2;
}

void store_vec(int* addr, int_vec value) {
    *addr = value;
}

#endif

__attribute__((noinline)) binary_search_result_t run_test(std::vector<int>& dataset1, std::vector<int>& dataset2, std::string test_name, int num_cores) {
    assert(dataset1.size() % VECTOR_SIZE == 0);
    assert(dataset1.size() == dataset2.size());

    int n = dataset1.size();
    int* v1 = dataset1.data();
    int* v2 = dataset2.data();

    binary_search_result_t result;

    std::string name = test_name + "_" + std::to_string(n) + "_" + std::to_string(num_cores);

    LIKWID_MARKER_START(name.c_str());

    int sum = 0;
    long mem_accesses = 0;

    #pragma omp parallel for reduction(+:sum, mem_accesses) num_threads(num_cores) schedule(static)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (v1[i] == v2[j]) {
                sum++;
            }
        }

        mem_accesses += n;
    }

    LIKWID_MARKER_STOP(name.c_str());

    result.memory_accesses = mem_accesses;
    result.index = sum;

    return result;
}


__attribute__((noinline)) binary_search_result_t run_test2(std::vector<int>& dataset1, std::vector<int>& dataset2, std::string test_name, int num_cores) {
    assert(dataset1.size() % VECTOR_SIZE == 0);
    assert(dataset2.size() % VECTOR_SIZE == 0);
    assert(dataset1.size() == dataset2.size());

    int n = dataset1.size();
    int* v1 = dataset1.data();
    int* v2 = dataset2.data();

    binary_search_result_t result;

    std::string name = test_name + "_" + std::to_string(n) + "_" + std::to_string(num_cores);

    LIKWID_MARKER_START(name.c_str());

    int sum = 0;
    long mem_accesses = 0;

    #pragma omp parallel num_threads(num_cores)
    {
        for (int i = 0; i < n; i++) {
            #pragma omp for reduction(+:sum, mem_accesses) schedule(static) nowait
            for (int j = 0; j < n; j++) {
                if (v1[i] == v2[j]) {
                    sum++;
                }
            }

            mem_accesses += n;
        }
    }

    LIKWID_MARKER_STOP(name.c_str());

    result.memory_accesses = mem_accesses;
    result.index = sum;

    return result;
}


int generate_data(int data_size, std::vector<int>& sorted_data) {
    sorted_data.resize(data_size * 2);
    std::iota(sorted_data.begin(), sorted_data.end(), 0);
    std::random_shuffle(sorted_data.begin(), sorted_data.end());
    sorted_data.resize(data_size);

    return data_size * 2;
}

void print(const std::vector<int>& v) {
    for (const auto& i: v) {
        std::cout << i << ", ";
    }
    std::cout << "\n";
}

int main(int argc, char** argv) {

    LIKWID_MARKER_INIT;
    static constexpr int start_size = 20*1000;
    static constexpr int end_size = 100*1024;
    static constexpr double diff = 1.41;

    int max_core_count = omp_get_max_threads();
    if (argc >= 2) {
        max_core_count = std::atoi(argv[1]);
    }
    std::cout << "Max cores = " << max_core_count << std::endl;

    for (int cores = 1; cores <= max_core_count; ++cores) {
        std::cout << "Running with " << cores << " cores\n";
        for (int size= start_size; size <= end_size; size *= diff) {
            binary_search_result_t res;
            std::vector<int> dataset1, dataset2;
            generate_data(size, dataset1);
            generate_data(size, dataset2);

            res = run_test2(dataset1, dataset2, "PARTITONED", cores);
            std::cout << "PARTITONED, cores = " << cores << ", size = " << size << ", " << "memory accesses = " 
                      << res.memory_accesses << ", found = " << res.index << "\n";


            res = run_test(dataset1, dataset2, "DEFAULT", cores);
            std::cout << "DEFAULT, cores = " << cores << ", size = " << size << ", " << "memory accesses = " 
                      << res.memory_accesses << ", found = " << res.index << "\n";
        }
    }

    LIKWID_MARKER_CLOSE;


    return 0;
}
