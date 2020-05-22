
#include "measure_time.h"
#include <iostream>
#include "linked_list.h"
#include "utils.h"

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

template <int size>
std::ostream& operator<<(std::ostream& os, const test_struct<size>& obj) {
    os << obj.my_val;
    return os;
}

template <typename test_struct, int size>
void __attribute__((noinline)) measure_find(linked_list<test_struct, size>& my_list, std::vector<int>& my_array, std::string message, int start, int stop, bool print_on_find) {
    measure_time m(message);
    bool find_result;

    for (int i = start; i < stop; i++) {
        find_result = my_list.find_if([i, &my_array] (const test_struct& x) -> bool { return x == my_array[i]; });
        if (print_on_find) {
            if (find_result) {
                std::cout << "Found";
            }
        } else {
            if (!find_result) {
                std::cout << "Not found";
            }
        }
    }
}

template<int linked_list_values, int iterations, int struct_size>
void run_test(std::vector<int>& my_array) {
    int len = my_array.size();

    for (int j = 0; j < iterations; j++) {
        linked_list<test_struct<struct_size>, linked_list_values> my_list;
        std::string header = "Node size = " + std::to_string(linked_list_values) + "|struct size = " + std::to_string(struct_size) + "|";
        {
            //measure_time m(header + "Emplace");
            for (int i = 0; i < len; i++ ) {
                my_list.emplace_back(my_array[i]);
            }
        }

        {
            //measure_time m(header + "remove_if");
            for (int i = 0; i < len / 2; i++) {
                my_list.remove_if([&my_array, i] (const test_struct<struct_size>& x) -> bool { return x == my_array[i]; });
            }
        }

        measure_find(my_list, my_array, header + " find if", 0, len / 2, true);
        measure_find(my_list, my_array, header + " find if2", len / 2, len, false);
    }

}

int main(int argc, char* argv[]) {
    constexpr int len = 50000;
    std::vector<int> my_array = create_growing_array(len, 1, len);
    //  std::vector<int> my_array = create_random_array(len, 1, len * 500);

    constexpr int iterations = 1;
    
    run_test<1, iterations, 1>(my_array);
    run_test<2, iterations, 1>(my_array);   
    run_test<4, iterations, 1>(my_array);
    run_test<8, iterations, 1>(my_array);

    run_test<1, iterations, 4>(my_array);
    run_test<2, iterations, 4>(my_array);   
    run_test<4, iterations, 4>(my_array);
    run_test<8, iterations, 4>(my_array);

    run_test<1, iterations, 8>(my_array);
    run_test<2, iterations, 8>(my_array);   
    run_test<4, iterations, 8>(my_array);
    run_test<8, iterations, 8>(my_array);

    measure_time_database<std::chrono::milliseconds>::get_instance()->dump_database();

    return 0;
}