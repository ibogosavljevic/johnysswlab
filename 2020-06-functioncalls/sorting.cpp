#include <algorithm>  
#include <iostream>
#include <cstring>

#include "utils.h"
#include "measure_time.h"

inline void __attribute__((always_inline))swap_if_smaller_inline(int* a, int *b) {
    if (*a < *b) {
        std::swap(*a, *b);
    }
}

void __attribute__((noinline))swap_if_smaller_noinline(int* a, int *b) {
    if (*a < *b) {
        std::swap(*a, *b);
    }
}

inline int __attribute__((always_inline)) find_min_element_inline(int* a, int start, int len) {
    int min = a[start];
    int min_index = start;
    for (int i = start + 1; i < len; i++) {
        if (a[i] < min) {
            min = a[i];
            min_index = i;
        }
    }
    return min_index;
}


int __attribute__((noinline)) find_min_element_noinline(int* a, int start, int len) {
    int min = a[start];
    int min_index = start;
    for (int i = start + 1; i < len; i++) {
        if (a[i] < min) {
            min = a[i];
            min_index = i;
        }
    }
    return min_index;
}


void sort_noinline_large(int* a, int len) {
    for (int i = 0; i < len; i++) {
        int smallest = find_min_element_noinline(a, i, len);
        std::swap(a[i], a[smallest]);
    }
}


void sort_inline_large(int* a, int len) {
    for (int i = 0; i < len; i++) {
        int smallest = find_min_element_inline(a, i, len);
        std::swap(a[i], a[smallest]);
    }
}


void sort_inline_small(int* a, int len) {
    for (int i = 0; i < len; i++) {
        for (int j = i+1; j < len; j++) {
            swap_if_smaller_inline(&a[j], &a[i]);
        }
    }
}

void sort_noinline_small(int* a, int len) {
    for (int i = 0; i < len; i++) {
        for (int j = i+1; j < len; j++) {
            swap_if_smaller_noinline(&a[j], &a[i]);
        }
    }
}


void sort_regular(int* a, int len) {
    for (int i = 0; i < len; i++) {
        for (int j = i+1; j < len; j++) {
            if (a[j] < a[i]) {
                std::swap(a[j], a[i]);
            }
        }
    }
}

static constexpr int arr_len = 40000;

int array_regular[arr_len];
int array_noinline_small[arr_len];
int array_inline_small[arr_len];
int array_noinline_large[arr_len];
int array_inline_large[arr_len];

int main(int argc, char* argv[]) {
    

    for (int i = 0; i < 5; i++) {
        std::vector<int> random_vector = create_random_array(arr_len, 0, arr_len);
        std::memcpy(array_regular, &random_vector[0], arr_len * sizeof(int));
        std::memcpy(array_noinline_small, &random_vector[0], arr_len * sizeof(int));
        std::memcpy(array_inline_small, &random_vector[0], arr_len * sizeof(int));
        std::memcpy(array_noinline_large, &random_vector[0], arr_len * sizeof(int));
        std::memcpy(array_inline_large, &random_vector[0], arr_len * sizeof(int));

        {
            measure_time m("sort function calling noinlined small function");
            sort_noinline_small(array_noinline_small, arr_len);
        }
        {
            measure_time m("sort function calling inlined small function");
            sort_inline_small(array_inline_small, arr_len);
        }
        {
            measure_time m("sort function calling noinlined large function");
            sort_noinline_large(array_noinline_large, arr_len);
        }
        {
            measure_time m("sort function calling inlined large function");
            sort_inline_large(array_inline_large, arr_len);
        }
        {
            measure_time m("regular sort function");
            sort_regular(array_regular, arr_len);
        }
    }

    measure_time_database<std::chrono::milliseconds>::get_instance()->dump_database();

    return 0;
}