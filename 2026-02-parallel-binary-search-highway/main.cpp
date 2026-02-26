#include <likwid.h>
#include <cmath>
#include <cassert>

#if defined(JSL_AVX)
#include "main-avx.cpp"
#elif defined(JSL_NEON)
#error "Gathers not supported on NEON" 
#elif defined(JSL_HIGHWAY)
#include "main-highway.cpp"
#else
#error "Not set"
#endif

#include "common.hpp"

void binary_search(int32_t* sorted, size_t sorted_size, int32_t* data, int32_t* found_idx, size_t data_size) {
    for (size_t i = 0; i < data_size; ++i) {
        int32_t key = data[i];
        found_idx[i] = -1;

        int low = 0;
        int high = sorted_size - 1;
        int mid;

        while(low <= high) {
            mid = (low + high) / 2;

            if (sorted[mid] < key) {
                low = mid + 1; 
            } else if(sorted[mid] > key) {
                high = mid - 1;
            } else {
                found_idx[i] = mid;
                break;
            }
        }
    }
}

void generate_sorted_array(int32_t* array, size_t size) {
    int32_t gen = 0;//100;
    for (size_t i = 0; i < size; ++i) {
        array[i] = gen;
        gen += rand() % 3 + 1;
    }
}

void generate_lookup_array(int32_t* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        data[i] = i;
    }

    for (size_t i = size - 1; i > 0; i--) { // for loop to shuffle
        size_t j = rand() % (i + 1); //randomise j for shuffle with Fisher Yates
        int32_t tmp = data[j];
        data[j] = data[i];
        data[i] = tmp;
    }
}

int main() {
    static constexpr size_t SORTED_SIZE = 1024*1024;
    static constexpr size_t DATA_SIZE =4*1024*1024;
    static constexpr size_t REPEAT_COUNT = 10;

    LIKWID_MARKER_INIT;

    int32_t* sorted = aligned::allocate_buffer<int32_t>(SORTED_SIZE);
    int32_t* data = aligned::allocate_buffer<int32_t>(DATA_SIZE);
    int32_t* found_idx1 = aligned::allocate_buffer<int32_t>(DATA_SIZE);
    int32_t* found_idx2 = aligned::allocate_buffer<int32_t>(DATA_SIZE);

    generate_sorted_array(sorted, SORTED_SIZE);
    generate_lookup_array(data, DATA_SIZE);
    set_buffer<int32_t>(found_idx1, DATA_SIZE, 0);
    set_buffer<int32_t>(found_idx2, DATA_SIZE, 0);

    run_test(REPEAT_COUNT, "compiler", [&]() -> void { binary_search(sorted, SORTED_SIZE, data, found_idx1, DATA_SIZE); });
    run_test(REPEAT_COUNT, "vectorized", [&]() -> void { binary_search_vectorized(sorted, SORTED_SIZE, data, found_idx2, DATA_SIZE); });

    equal(found_idx1, found_idx2, DATA_SIZE);

    aligned::free_buffer(sorted);
    aligned::free_buffer(data);
    aligned::free_buffer(found_idx1);
    aligned::free_buffer(found_idx2);

    LIKWID_MARKER_CLOSE;
}