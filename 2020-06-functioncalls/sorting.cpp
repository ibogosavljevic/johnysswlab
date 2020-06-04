#include <algorithm>  
#include <iostream>
#include <cstring>

#include "utils.h"
#include "measure_time.h"

inline void __attribute__((always_inline))update_min_index_inline(int* current, int current_index, int *min, int* min_index) {
    if (*current < *min) {
        *min = *current;
        *min_index = current_index;
    }
}

inline void __attribute__((noinline))update_min_index_noinline(int* current, int current_index, int *min, int* min_index) {
    if (*current < *min) {
        *min = *current;
        *min_index = current_index;
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
        int min = a[i];
        int min_index = i;
        for (int j = i+1; j < len; j++) {
            update_min_index_inline(&a[j], j, &min, &min_index);
        }
        std::swap(a[i], a[min_index]);
    }
}

void sort_noinline_small(int* a, int len) {
    for (int i = 0; i < len; i++) {
        int min = a[i];
        int min_index = i;
        for (int j = i+1; j < len; j++) {
            update_min_index_noinline(&a[j], j, &min, &min_index);
        }
        std::swap(a[i], a[min_index]);
    }
}


void sort_regular(int* a, int len) {
    for (int i = 0; i < len; i++) {
        int min = a[i];
        int min_index = i;
        for (int j = i+1; j < len; j++) {
            if (a[j] < min) {
                min = a[j];
                min_index = j;
            }
        }
        std::swap(a[i], a[min_index]);
    }
}

void verify_sorted(int* a, int len, const char* msg) {
    for (int i = 1; i < len; i++) {
        if (a[i] < a[i - 1]) {
            printf("%s not sorted\n", msg);
            return;
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

        verify_sorted(array_noinline_small, arr_len, "noinline small");
        verify_sorted(array_inline_small, arr_len, "inline small");
        verify_sorted(array_noinline_large, arr_len, "noinline large");
        verify_sorted(array_inline_large, arr_len, "inline large");
        verify_sorted(array_regular, arr_len, "regular");
    }

    measure_time_database<std::chrono::milliseconds>::get_instance()->dump_database();

    return 0;
}