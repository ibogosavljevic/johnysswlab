
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <iostream>
#include "likwid.h"

int binary_search(int* array, int number_of_elements, int key) {
    int low = 0, high = number_of_elements-1, mid;
    while(low <= high) {
        mid = (low + high)/2;

        if(array[mid] < key) {
            low = mid + 1; 
        } else if (array[mid] > key) {
            high = mid-1; 
        } else {
            return mid;
        }
    }
    return -1;
}

int binary_search_branchless(int* array, int number_of_elements, int key) {
    int low = 0, high = number_of_elements-1, mid;
    while(low <= high) {
        mid = (low + high)/2;

        int mid_value = array[mid];

        if (mid_value == key) {
            return mid;
        }

        int new_low = mid + 1;
        int new_high = mid - 1;

        bool cond_less = mid_value < key;

        low = cond_less ? new_low: low;
        high = cond_less ? high: new_high;

    }
    return -1;
}

int binary_search_bracnhless_asm(int* array, int number_of_elements, int key) {
    int low = 0, high = number_of_elements-1, mid;
    while(low <= high) {
        mid = (low + high)/2;

        int mid_value = array[mid];

        if (mid_value == key) {
            return mid;
        }

        int new_low = mid + 1;
        int new_high = mid - 1;

#if defined(__x86_64)
            __asm__ (
                "cmp %[mid_value], %[key];"
                "cmovae %[new_low], %[low];"
                "cmovb %[new_high], %[high];"
                : [low] "+&r"(low), [high] "+&r"(high)
                : [new_low] "r"(new_low), [new_high] "r"(new_high), [mid_value] "r"(mid_value), [key] "r"(key)
                : "cc"
            );
#elif defined(__aarch64__)
            __asm__ (
                "cmp %[array_middle], %[key];"
                "csel %[low], %[new_low], %[low], le;"
                "csel %[high], %[new_high], %[high], gt;"
                : [low] "+&r"(low), [high] "+&r"(high)
                : [new_low] "r"(new_low), [new_high] "r"(new_high), [array_middle] "r"(mid_value), [key] "r"(key)
                : "cc"
            );
#else
#error Unsupported archtecture
#endif

        bool cond_less = mid_value < key;

        low = cond_less ? new_low: low;
        high = cond_less ? high: new_high;

    }
    return -1;
}




int main() { 
    int start_size = 1024;
    int end_size = 32 * 1024 * 1024;
    int lookup_cnt = end_size * 2;

    LIKWID_MARKER_INIT;

    for (int size = start_size; size <= end_size; size *= 2) {
        std::vector<int> result1(lookup_cnt, 0);
        std::vector<int> result2(lookup_cnt, 0);
        std::vector<int> input_data(lookup_cnt);
        std::vector<int> sorted_data(size);

        std::iota(input_data.begin(), input_data.end(), 0);
        std::random_shuffle(input_data.begin(), input_data.end());
        for (int i = 0; i < size; i++) {
            sorted_data[i] = input_data[i];
        }

        std::sort(sorted_data.begin(), sorted_data.end());
        std::random_shuffle(input_data.begin(), input_data.end());

        std::string branchfull = "BRANCHFULL_" + std::to_string(size);
        std::string branchless = "BRANCHLESS_" + std::to_string(size);

        std::cout << "size = " << size << std::endl;

        LIKWID_MARKER_START(branchfull.c_str());
        for (int i = 0; i < input_data.size(); i++) {
            result1[i] += binary_search(&sorted_data[0], size, input_data[i]) != -1;
        }
        LIKWID_MARKER_STOP(branchfull.c_str());

        LIKWID_MARKER_START(branchless.c_str());
        for (int i = 0; i < input_data.size(); i++) {
            result2[i] += binary_search_bracnhless_asm(&sorted_data[0], size, input_data[i]) != -1;
        }
        LIKWID_MARKER_STOP(branchless.c_str());


        if (result1 != result2) {
            std::cout << "Error\n";
        }

    }

    LIKWID_MARKER_CLOSE;
    
    return 0; 
}
