#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>

#include "hybrid_linked_list.h"
#include "likwid.h"

static constexpr int SIZE = 16 * 1024 * 1024;
static constexpr int LOOKUP_SIZE = 128;

int main(int argc, char **argv) {
    LIKWID_MARKER_INIT;

    hybrid_linked_list<int> test_list(SIZE);
    std::vector<int> test_values(SIZE);
    std::vector<int> lookup_values(LOOKUP_SIZE);

    std::iota(test_values.begin(), test_values.end(), 0);
    std::random_shuffle(test_values.begin(), test_values.end());

    std::iota(lookup_values.begin(), lookup_values.end(), -LOOKUP_SIZE/2);
    std::random_shuffle(lookup_values.begin(), lookup_values.end());

    for (const auto& v: test_values) {
        bool result = test_list.push_back(v);
        if (!result) {
            std::abort();
        }
    }

    test_list.shuffle_list(SIZE / 4);

    LIKWID_MARKER_START("Lookup_Simple");
    std::vector<bool> lookup_result_simple = test_list.lookup_values_simple(lookup_values);
    LIKWID_MARKER_STOP("Lookup_Simple");

    LIKWID_MARKER_START("Lookup_Interleaved");
    std::vector<bool> lookup_result_interleaved = test_list.lookup_values_interleaved(lookup_values);
    LIKWID_MARKER_STOP("Lookup_Interleaved");

    LIKWID_MARKER_START("Lookup_Array");
    std::vector<bool> lookup_result_array = test_list.lookup_values_array(lookup_values);
    LIKWID_MARKER_STOP("Lookup_Array");

    LIKWID_MARKER_START("Lookup_Index");
    std::vector<bool> lookup_result_index = test_list.lookup_values_index(lookup_values);
    LIKWID_MARKER_STOP("Lookup_Index");

    if (   lookup_result_interleaved == lookup_result_simple && 
           lookup_result_array == lookup_result_interleaved &&
           lookup_result_interleaved == lookup_result_index) {
        std::cout << "Done\n";
    } else {
        std::cout << "Result mismatch\n";
    }

    LIKWID_MARKER_CLOSE;

    return 0;
}