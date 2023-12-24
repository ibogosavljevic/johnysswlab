#include "utils.h"

#include <iostream>
#include <vector>
#include <cassert>

#include "likwid.h"


enum search_type {
   REGULAR,
   CONDITIONAL_MOVE,
   ARITHMETIC,
   UNKNOWN,
};



template <search_type st>
int binary_search(int* array, int number_of_elements, int key) {
    int low = 0, high = number_of_elements-1, mid;
    while(low <= high) {
        mid = (low + high)/2;

        if (st == search_type::REGULAR) {
            if(array[mid] < key)
                low = mid + 1; 
            else if(array[mid] == key)
                return mid;
            else
                high = mid-1;
        }

        if (st == search_type::CONDITIONAL_MOVE) {
            int middle = array[mid];
            if (middle == key) {
                    return mid;
                }

            int new_low = mid + 1;
            int new_high = mid - 1;
            __asm__ (
                "cmp %[array_middle], %[key];"
                "cmovae %[new_low], %[low];"
                "cmovb %[new_high], %[high];"
                : [low] "+&r"(low), [high] "+&r"(high)
                : [new_low] "g"(new_low), [new_high] "g"(new_high), [array_middle] "g"(middle), [key] "g"(key)
                : "cc"
            );
        }

        if (st == search_type::ARITHMETIC) {
            int middle = array[mid];
            if (middle == key) {
                return mid;
            }

            int new_low = mid + 1;
            int new_high = mid - 1;
            int condition = array[mid] < key;
            int condition_true_mask = -condition;
            int condition_false_mask = -(1 - condition);

            low += condition_true_mask & (new_low - low);
            high += condition_false_mask & (new_high - high); 

        }
    }
    return -1;
}

int main(int argc, const char* argv[]) {
    static constexpr size_t start_size = 4*1024;
    static constexpr size_t end_size = 64 * 1024 * 1024;

    static constexpr size_t lookups = 4 * 1024 * 1024;

    LIKWID_MARKER_INIT;

    for (size_t s = start_size; s <= end_size; s *= 4) {
        size_t size = s - 674;
        int * input_array = (int*) malloc(sizeof(int) * size);
        generate_random_growing_array(input_array, size);

        std::vector<int> index_array = create_random_array<int>(lookups, input_array[0] - 100, input_array[size-1] + 100);
        std::string prefix = "SIZE_" + std::to_string(s);
        
        std::string desc1 = prefix + "_BRANCHFUL";
        int i1 = 0;
        LIKWID_MARKER_START(desc1.c_str());
        for (size_t i = 0; i < lookups; i++) {
            i1 += binary_search<search_type::REGULAR>(input_array, size, index_array[i]);
        }
        LIKWID_MARKER_STOP(desc1.c_str());

        std::string desc2 = prefix + "_CONDMOVE";
        int i2 = 0;
        LIKWID_MARKER_START(desc2.c_str());
        for (size_t i = 0; i < lookups; i++) {
            i2 += binary_search<search_type::CONDITIONAL_MOVE>(input_array, size, index_array[i]);
        }
        LIKWID_MARKER_STOP(desc2.c_str());

        std::string desc3 = prefix + "_ARITHMETIC";
        int i3 = 0;
        LIKWID_MARKER_START(desc3.c_str());
        for (size_t i = 0; i < lookups; i++) {
            i3 += binary_search<search_type::ARITHMETIC>(input_array, size, index_array[i]);
        }
        LIKWID_MARKER_STOP(desc3.c_str());


        free(input_array);

        assert(i1 == i2);
        assert(i1 == i3);


    }

    LIKWID_MARKER_CLOSE;

    return 0;
}
