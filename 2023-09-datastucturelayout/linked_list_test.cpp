
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include "linked_list.h"
#include <likwid.h>

template <int size>
struct test_struct {
    int my_val[size];

    test_struct(int val) {
        my_val[0] = val;
    }

    bool operator==(const test_struct& rhs) const {
        return my_val[0] == rhs.my_val[0];
    }

    bool operator==(const int& rhs) {
        return my_val[0] == rhs;
    }
};

template <typename It>
void random_shuffle(It begin, It end) {
    std::random_device rng;
    std::mt19937 urng(rng());
    std::shuffle(begin, end, urng);
}

template <int size>
std::ostream& operator<<(std::ostream& os, const test_struct<size>& obj) {
    os << obj.my_val;
    return os;
}

template <typename test_struct, int size>
void measure_find(std::string message, linked_list<test_struct, size>& my_list, std::vector<int>& my_array) {
    int found_count = 0;
    LIKWID_MARKER_START(message.c_str());

    for (int i = 0; i < my_array.size(); i++) {
        bool find_result = my_list.find_if([i, &my_array] (const test_struct& x) -> bool { return x == my_array[i]; });
        found_count += (find_result == true);
    }
    LIKWID_MARKER_STOP(message.c_str());

    std::cout << message << ", found count = " << found_count << std::endl;
}

template<int linked_list_values, int struct_size>
void run_test(std::vector<int>& my_array, bool shuffle_list) {
    int len = my_array.size();

    linked_list<test_struct<struct_size>, linked_list_values> my_list;

    std::string header = "Node_" + std::to_string(linked_list_values) + "_struct_" + std::to_string(struct_size) + "_shuffle_" + std::to_string(shuffle_list);
    std::cout << "Node size = " << my_list.get_node_size() << std::endl;

    // Create the list
    for (int i = 0; i < len; i++ ) {
        my_list.emplace_back(my_array[i]);
    }

    ::random_shuffle(my_array.begin(), my_array.end());
    if (shuffle_list) {
        my_list.shuffle();
    }

    // Remove one quarted of all the data
    for (int i = 0; i < len / 4; i++) {
        my_list.remove_if([&my_array, i] (const test_struct<struct_size>& x) -> bool { return x == my_array[i]; });
    }

    ::random_shuffle(my_array.begin(), my_array.end());

    measure_find(header, my_list, my_array);
}

int main(int argc, char* argv[]) {
    constexpr int len = 50000;
    std::vector<int> test_data(len);
    std::iota(test_data.begin(), test_data.end(), 0);
    ::random_shuffle(test_data.begin(), test_data.end());

    LIKWID_MARKER_INIT;

    run_test<1, 4>(test_data, false);
    run_test<2, 4>(test_data, false);
    run_test<4, 4>(test_data, false);
    run_test<8, 4>(test_data, false);

    run_test<1, 4>(test_data, true);
    run_test<2, 4>(test_data, true);
    run_test<4, 4>(test_data, true);
    run_test<8, 4>(test_data, true);

    LIKWID_MARKER_CLOSE;

    return 0;
}