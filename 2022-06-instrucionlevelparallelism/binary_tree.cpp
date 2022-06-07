#include <set>
#include <iostream>
#include "binary_tree.h"
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

using simple_binary_tree_t = binary_tree<int, simple_binary_tree_node<int>, Moya::MemoryPool<simple_binary_tree_node<int>> >;
using vector_backed_binary_tree_t = binary_tree<int, vector_backed_binary_tree_node<int>, std::vector<vector_backed_binary_tree_node<int>>>;


void run_test(size_t size, size_t repeat_count) {
    std::vector<int> test_arr = generate_sorted_array(size);

    std::string simple_name = "Simple_" + std::to_string(size);
    std::string interleaved_name = "Simple_" + std::to_string(size);
    std::string verify_name = "Verify_" + std::to_string(size);

    simple_binary_tree_t simple_binary_tree = simple_binary_tree_t::build_tree(test_arr);
    std::set<int> verify_tree;

    for (const auto& val: test_arr) {
        verify_tree.insert(val);
    }

    std::cout << "Sizeof(simple_tree) = " << simple_binary_tree_t::get_size_of_chunk();

    int end = test_arr[test_arr.size() - 1] + 100;

    std::vector<int> lookup_arr(end + 100);
    std::iota(lookup_arr.begin(), lookup_arr.end(), -100);
    std::random_shuffle(lookup_arr.begin(), lookup_arr.end());

    std::vector<bool> simple_results;
    std::vector<bool> interleaved_results;
    std::vector<bool> verify_results(lookup_arr.size(), false);

    std::cout << "Running, size = " << size << ", repeat count = " << repeat_count << "\n";

    for (int i = 0; i < repeat_count; i++) {
        LIKWID_MARKER_START(simple_name.c_str());
        simple_results = simple_binary_tree.find_all(lookup_arr);
        LIKWID_MARKER_STOP(simple_name.c_str());
    }

    for (int i = 0; i < repeat_count; i++) {
        LIKWID_MARKER_START(interleaved_name.c_str());
        interleaved_results = simple_binary_tree.find_all_interleaved(lookup_arr);
        LIKWID_MARKER_STOP(interleaved_name.c_str());
    }

    verify_results.reserve(lookup_arr.size());

    for (int i = 0; i < repeat_count; i++) {
        LIKWID_MARKER_START(verify_name.c_str());
        for (int j = 0; j < lookup_arr.size(); j++) {
            verify_results[j] = (verify_tree.find(lookup_arr[j]) != verify_tree.end());
        }
        LIKWID_MARKER_STOP(verify_name.c_str());
    }

    if (verify_results != simple_results || verify_results != interleaved_results) {
        std::cout << "Result mismatch\n";
    } else {
        std::cout << "Done\n";
    }
}

int main(int argc, char* argv[]) {
    LIKWID_MARKER_INIT;

    static constexpr size_t end_size = 16 * 1024 * 1024;
    static constexpr size_t start_size = 8 * 1024;

    for (size_t size = start_size; size <= end_size; size *= 2) {
        size_t repeat_count = end_size / size;
        run_test(size, repeat_count);
    }

    LIKWID_MARKER_CLOSE;
 }