#include <algorithm>
#include <cstdlib>
#include <iostream>
#include "measure_time.h"
#include "utils.h"

int compare_int(const void* a, const void* b) {
    return *(int*)a < *(int*)b;
}

#define ARR_SIZE (10 * 1024 * 1024)

int main(int argc, char** argv) {
    std::vector<int> my_random_ints =
        create_random_array<int>(ARR_SIZE, 0, ARR_SIZE);
    std::vector<int> my_random_ints2 = my_random_ints;

    {
        measure_time m("qsort");
        qsort(&my_random_ints[0], ARR_SIZE, sizeof(int), compare_int);
    }

    {
        measure_time m("std::sort");
        std::sort(my_random_ints2.begin(), my_random_ints2.end(),
                  [](const int i1, const int i2) { return i1 > i2; });
    }

    if (my_random_ints != my_random_ints2) {
        std::cout << "Arrays different\n";
    }

    return 0;
}