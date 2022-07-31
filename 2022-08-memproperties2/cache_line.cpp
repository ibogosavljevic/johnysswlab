#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <iostream>
#include "likwid.h"

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

static constexpr int TEST_SIZE = 8*1024*1024;

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

int main(int argc, char **argv) {
    LIKWID_MARKER_INIT;

    int start_size = 64;
    int end_size = TEST_SIZE;

    for (int size = start_size; size <= end_size; size *= 2) {
        std::vector<int> index_vector(size);

        std::iota(index_vector.begin(), index_vector.end(), 0);
        std::random_shuffle(index_vector.begin(), index_vector.end());

        test_struct<CACHE_LINE_SIZE> * aligned_struct = allocate_aligned_struct(size);
        test_struct<CACHE_LINE_SIZE> * unaligned_struct = allocate_unaligned_struct(size);

        std::cout << aligned_struct << std::endl;
        std::cout << unaligned_struct << std::endl;
        std::cout << "Running for size = " << size << std::endl;

        int repeat_count = TEST_SIZE / size * 4;
        int sum1, sum2;

        std::string init_aligned_text = "init_aligned_" + std::to_string(size);
        std::string init_unaligned_text = "init_unaligned_" + std::to_string(size);
        std::string sum_aligned_text = "sum_aligned_" + std::to_string(size);
        std::string sum_unaligned_text = "sum_unaligned_" + std::to_string(size);

        LIKWID_MARKER_START(init_aligned_text.c_str());
        for (int i = 0; i < repeat_count; i++) {
            initialize_struct(aligned_struct, size);
            clobber();
        }
        LIKWID_MARKER_STOP(init_aligned_text.c_str());


        LIKWID_MARKER_START(init_unaligned_text.c_str());
        for (int i = 0; i < repeat_count; i++) {
            initialize_struct(unaligned_struct, size);
            clobber();
        }
        LIKWID_MARKER_STOP(init_unaligned_text.c_str());


        LIKWID_MARKER_START(sum_aligned_text.c_str());
        for (int i = 0; i < repeat_count; i++) {
            sum1 = sum_struct(aligned_struct, size, index_vector);
            clobber();
        }
        LIKWID_MARKER_STOP(sum_aligned_text.c_str());
        
        LIKWID_MARKER_START(sum_unaligned_text.c_str());
        for (int i = 0; i < repeat_count; i++) {
            sum2 = sum_struct(unaligned_struct, size, index_vector);
            clobber();
        }
        LIKWID_MARKER_STOP(sum_unaligned_text.c_str());

        if (sum1 != sum2) {
            std::cout << "Unexpected\n";
        }
   
        free_struct(aligned_struct);
        free_struct(unaligned_struct);
    }
    
    LIKWID_MARKER_CLOSE;

    return 0;
}