#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <string>

#include "hybrid_linked_list.h"
#include "likwid.h"

static constexpr int SIZE = 16 * 1024 * 1024;
static constexpr int LOOKUP_SIZE = 128;

void run_test(hybrid_linked_list<int>& test_list, std::vector<int>& lookup_values, std::string suffix) {
    std::string simple_tag = "Layout_Simple_" + suffix;
    std::string interleaved_tag = "Layout_Interleaved_" + suffix;
    std::string array_tag = "Layout_Array_" + suffix;
    std::string index_tag = "Layout_Index_" + suffix;

    LIKWID_MARKER_START(simple_tag.c_str());
    std::vector<bool> lookup_result_simple = test_list.lookup_values_simple(lookup_values);
    LIKWID_MARKER_STOP(simple_tag.c_str());

    LIKWID_MARKER_START(interleaved_tag.c_str());
    std::vector<bool> lookup_result_interleaved = test_list.lookup_values_interleaved(lookup_values);
    LIKWID_MARKER_STOP(interleaved_tag.c_str());

    LIKWID_MARKER_START(array_tag.c_str());
    std::vector<bool> lookup_result_array = test_list.lookup_values_array(lookup_values);
    LIKWID_MARKER_STOP(array_tag.c_str());

    LIKWID_MARKER_START(index_tag.c_str());
    std::vector<bool> lookup_result_index = test_list.lookup_values_index(lookup_values);
    LIKWID_MARKER_STOP(index_tag.c_str());

    if (   lookup_result_interleaved == lookup_result_simple && 
           lookup_result_array == lookup_result_interleaved &&
           lookup_result_interleaved == lookup_result_index) {
        std::cout << "Done\n";
    } else {
        std::cout << "Result mismatch\n";
    }


}

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

    run_test(test_list, lookup_values, "Perfect");
    
    test_list.shuffle_list(SIZE / 4);
    run_test(test_list, lookup_values, "Misplaced_25_Percent");

    LIKWID_MARKER_CLOSE;

    return 0;
}