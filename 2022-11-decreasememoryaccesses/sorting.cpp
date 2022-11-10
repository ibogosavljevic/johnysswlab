#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>

#include "likwid.h"

void selection_sort_simple(double* a, size_t n) {
    for (size_t i = 0; i < n; i++) {
        double min = a[i];
        size_t index_min = i;
        for (size_t j = i + 1; j < n; j++) {
            if (a[j] < min) {
                min = a[j];
                index_min = j;
            }
        }

        std::swap(a[i], a[index_min]);
    }
}

void selection_sort_double(double* a, size_t n) {
    for (size_t i = 0, j = n - 1; i < j; i++, j--) {
        double min = a[i];
        size_t index_min = i;
        double max = a[j];
        size_t index_max = j;
        for (size_t k = i; k <= j; k++) {
            if (a[k] < min) {
                min = a[k];
                index_min = k;
            }
            if (a[k] > max) {
                max = a[k];
                index_max = k;
            }
        }

        std::swap(a[i], a[index_min]);

        if (a[index_min] != max) {
            std::swap(a[j], a[index_max]);
        } else {
            std::swap(a[j], a[index_min]);
        }
    }
}

std::vector<double> generate_random_numbers(size_t size) {
    std::vector<double> result;
    result.reserve(size*2);
    double v = 0.0;
    for (size_t i = 0; i < (size*2); i++) {
        result.push_back(v);

        if ((static_cast<size_t>(i + v) % 3) != 0) {
            v += 0.1;
        }
    }

    std::random_shuffle(result.begin(), result.end());
    result.resize(size);

    return result;
}



int main(int argc, char** argv) {
    static constexpr size_t start_size = 1024;
    static constexpr size_t end_size = 64 * 1024;
    static constexpr size_t complexity = end_size * end_size; 

    LIKWID_MARKER_INIT;

    for (size_t size = start_size; size <= end_size; size *= 2) {
        std::cout << "SIZE = " << size << std::endl;
        std::vector<double> input = generate_random_numbers(size);
        size_t repeat_count = 4 * complexity / (size * size);

        std::string name1 = "simple_" + std::to_string(size);
        std::vector<double> in1;
        for (size_t i = 0; i < repeat_count; i++) {
            in1 = input;
            LIKWID_MARKER_START(name1.c_str());
            selection_sort_simple(in1.data(), in1.size());
            LIKWID_MARKER_STOP(name1.c_str());
        }

        std::vector<double> in2;
        std::string name2 = "double_" + std::to_string(size);
        for (size_t i = 0; i < repeat_count; i++) {
            in2 = input;
            LIKWID_MARKER_START(name2.c_str());
            selection_sort_double(in2.data(), in2.size());
            LIKWID_MARKER_STOP(name2.c_str());
        }

        std::sort(input.begin(), input.end());
        if (in1 != input) {
            std::cout << "ERR1\n";
        }

        if (in2 != input) {
            std::cout << "ERR2\n";
        }
    }

    LIKWID_MARKER_CLOSE;

    return 0;
}