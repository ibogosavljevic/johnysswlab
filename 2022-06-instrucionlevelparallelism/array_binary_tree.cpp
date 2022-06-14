#include <vector>
#include <cstdlib>
#include <string>
#include <limits>
#include <set>
#include <iostream>
#include <algorithm>
#include <numeric>

#include "array_binary_tree.h"
#include "likwid.h"


std::vector<int> generate_sorted_array(size_t size) {
    std::vector<int> result;

    size_t number_added = 0;
    int current = 0;

    while (number_added < size) {
        int random_number = rand();
        
        if (random_number % 4 == 0) {
            result.push_back(current);
            number_added++;
        }
        
        current++;
    }
    
    return result;
}


void run_test(size_t size, size_t repeat_count) {
    std::vector<int> test_arr = generate_sorted_array(size);
    array_binary_tree<int> test_tree = array_binary_tree<int>::build_tree(test_arr, std::numeric_limits<int>::min());

    std::string simple_name = "Simple_" + std::to_string(size);
    std::string array_name = "Array_" + std::to_string(size);
    std::string verify_name = "Verify_" + std::to_string(size);

    std::set<int> verify_tree;

    for (const auto& val: test_arr) {
        verify_tree.insert(val);
    }

    int end = test_arr[test_arr.size() - 1] + 100;

    std::vector<int> lookup_arr(end + 100);
    std::iota(lookup_arr.begin(), lookup_arr.end(), -100);
    std::random_shuffle(lookup_arr.begin(), lookup_arr.end());

    std::vector<bool> simple_results(lookup_arr.size(), false);
    std::vector<bool> array_results(lookup_arr.size(), false);
    std::vector<bool> verify_results(lookup_arr.size(), false);

    std::cout << "Running, size = " << size << ", repeat count = " << repeat_count << "\n";

    for (int i = 0; i < repeat_count; i++) {
        LIKWID_MARKER_START(simple_name.c_str());
        for (int j = 0; j < lookup_arr.size(); j++) {
            simple_results[j] = test_tree.find(lookup_arr[j]);
        }
        LIKWID_MARKER_STOP(simple_name.c_str());
    }

    for (int i = 0; i < repeat_count; i++) {
        LIKWID_MARKER_START(array_name.c_str());
        for (int j = 0; j < lookup_arr.size(); j++) {
            array_results[j] = test_tree.find_array(lookup_arr[j]);
        }
        LIKWID_MARKER_STOP(array_name.c_str());
    }

    for (int i = 0; i < repeat_count; i++) {
        LIKWID_MARKER_START(verify_name.c_str());
        for (int j = 0; j < lookup_arr.size(); j++) {
            verify_results[j] = (verify_tree.find(lookup_arr[j]) != verify_tree.end());
        }
        LIKWID_MARKER_STOP(verify_name.c_str());
    }

    if (verify_results != simple_results || verify_results != array_results) {
        std::cout << "Result mismatch\n";
    } else {
        std::cout << "Done\n";
    }
}



int main(int argc, char* argv[]) {
    LIKWID_MARKER_INIT;

    std::vector<int> test = { 1, 2, 3, 4, 5, 6 };
    array_binary_tree<int> test_tree = array_binary_tree<int>::build_tree(test, std::numeric_limits<int>::min());
    test_tree.find_array(0);
    test_tree.find_array(3);
    test_tree.find_array(7);

    static constexpr size_t end_size = 16 * 1024 * 1024;
    static constexpr size_t start_size = 8 * 1024;

    for (size_t size = start_size; size <= end_size; size *= 2) {
        size_t repeat_count = end_size / size;
        run_test(size, repeat_count);
    }

    LIKWID_MARKER_CLOSE;
}