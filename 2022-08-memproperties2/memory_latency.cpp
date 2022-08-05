#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iostream>

#include "likwid.h"

struct elem_t {
    elem_t* next;
    double value;
};

std::vector<elem_t> generate_linked_list(int size, elem_t** head) {
    std::vector<elem_t> result(size);
    std::vector<int> position(size);

    std::iota(position.begin(), position.end(), 0);
    std::random_shuffle(position.begin(), position.end());

    for (int i = 0; i < size; i++) {
        result[i].value = 1.0 / i;
    }

    *head = &result[position[0]];

    for (int i = 0; i < size - 1; i++) {
        result[position[i]].next = &result[position[i + 1]];
    }

    result[position[size - 1]].next = nullptr;

    return result;
}

double calculate_min_sum(std::vector<elem_t*> heads) {
    std::vector<elem_t*> currents = heads;
    std::vector<double> sums(heads.size(), 0.0);
    int size = heads.size();

    while(currents[0] != nullptr) {
        for (int i = 0; i < size; i++) {
            sums[i] += i * currents[i]->value;
            currents[i] = currents[i]->next;
        }
    }

    double min = sums[0];
    for (int i = 0; i < size; i++) {
        min = std::min(sums[i], min);
    }

    return min;
}

static void escape(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}

static void clobber() {
    asm volatile("" : : : "memory");
}


int main(int argc, char** argv) {

    int start_size = 1024;
    int end_size = 64 * 1024 * 1024;

    LIKWID_MARKER_INIT;


    for (int list_count = 1; list_count < 12; list_count++) {
        for (int size = start_size; size <= end_size; size *= 2) {
            int repeat_count = end_size / size * 4;

            std::vector<std::vector<elem_t>> lists;
            std::vector<elem_t*> heads;
            std::string test_name = "CALC_LC_" + std::to_string(list_count) + "_SIZE_" + std::to_string(size) + "_REPEATS_" + std::to_string(repeat_count);

            for (int i = 0; i < list_count; i++) {
                elem_t* head;
                lists.emplace_back(generate_linked_list(size, &head));
                heads.emplace_back(head);
            }


            double min = 0.0;
            LIKWID_MARKER_START(test_name.c_str());
            for (int i = 0; i  < repeat_count; i++) {
                min += calculate_min_sum(heads);
                clobber();
            }
            LIKWID_MARKER_STOP(test_name.c_str());

            std::cout << "list count = " << list_count << ", size = " << size << std::endl;


        }
    }

    LIKWID_MARKER_CLOSE;
}

