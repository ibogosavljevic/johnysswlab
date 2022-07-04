#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <iostream>
#include "likwid.h"

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

template <int CacheLineSize = CACHE_LINE_SIZE>
struct test_struct {
    unsigned char first_byte;
    unsigned char padding[CACHE_LINE_SIZE - 2];
    unsigned char last_byte;
};


test_struct<CACHE_LINE_SIZE> * allocate_aligned_struct(int size) {
    void* result;
    posix_memalign( &result, CACHE_LINE_SIZE, size * sizeof(test_struct<CACHE_LINE_SIZE>));
    return (test_struct<CACHE_LINE_SIZE> *) result;
}

test_struct<CACHE_LINE_SIZE> * allocate_unaligned_struct(int size) {
    void* result;
    posix_memalign( &result, CACHE_LINE_SIZE, (size + 1) * sizeof(test_struct<CACHE_LINE_SIZE>));
    char* result2 = (char*) result;
    result2++;
    return (test_struct<CACHE_LINE_SIZE> *) result2;
}

void free_struct(test_struct<CACHE_LINE_SIZE> * p) {
    uintptr_t p_val = (uintptr_t) p;
    if (p_val % CACHE_LINE_SIZE == 0) {
        free(p);
    } else {
        char* p2 = (char*) p;
        p2--;
        free(p2);
    }
}

void initialize_struct(test_struct<CACHE_LINE_SIZE> * p, int n) {
    for (int i = 0; i < n; i++) {
        p[i].first_byte = i;
        p[i].last_byte = n - i;
    }
}

int sum_struct(test_struct<CACHE_LINE_SIZE> * p, int n, std::vector<int>& index_array) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        int index = index_array[i];
        sum += p[index].first_byte + p[index].last_byte;
    }

    return sum;
}

static void clobber() {
    asm volatile("" : : : "memory");
}

static constexpr int TEST_SIZE = 1*1024*1024;

int main(int argc, char **argv) {
    std::vector<int> index_vector(TEST_SIZE);
    std::iota(index_vector.begin(), index_vector.end(), 0);
    std::random_shuffle(index_vector.begin(), index_vector.end());

    test_struct<CACHE_LINE_SIZE> * aligned_struct = allocate_aligned_struct(TEST_SIZE);
    test_struct<CACHE_LINE_SIZE> * unaligned_struct = allocate_unaligned_struct(TEST_SIZE);

    std::cout << aligned_struct << std::endl;
    std::cout << unaligned_struct << std::endl;

    LIKWID_MARKER_INIT;

    LIKWID_MARKER_START("init_aligned");
    initialize_struct(aligned_struct, TEST_SIZE);
    LIKWID_MARKER_STOP("init_aligned");
    
    LIKWID_MARKER_START("init_unaligned");
    initialize_struct(unaligned_struct, TEST_SIZE);
    LIKWID_MARKER_STOP("init_unaligned");

    LIKWID_MARKER_START("sum_aligned");
    int sum1 = sum_struct(aligned_struct, TEST_SIZE, index_vector);
    LIKWID_MARKER_STOP("sum_aligned");
    
    LIKWID_MARKER_START("sum_unaligned");
    int sum2 = sum_struct(unaligned_struct, TEST_SIZE, index_vector);
    LIKWID_MARKER_STOP("sum_unaligned");

    clobber();

    std::cout << sum1 << ", " << sum2 << std::endl;

    free_struct(aligned_struct);
    free_struct(unaligned_struct);

    LIKWID_MARKER_CLOSE;

    return 0;
}