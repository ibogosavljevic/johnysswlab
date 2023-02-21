#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include "likwid.h"

template<typename T, typename It>
T find_min(It begin, It end) {
    T min = *begin;
    for (It it = begin; it != end; ++it) {
        if (*it < min) {
            min = *it;
        }
    }
    return min;
}

template<typename T, typename It>
T find_max(It begin, It end) {
    T max = *begin;
    for (It it = begin; it != end; ++it) {
        if (*it > max) {
            max = *it;
        }
    }
    return max;
}

template<typename T, typename It>
std::pair<T, T> find_minmax(It begin, It end) {
    T min = *begin;
    T max = *begin;
    for (It it = begin; it != end; ++it) {
        if (*it < min) {
            min = *it;
        }
        if (*it > max) {
            max = *it;
        }
    }
    return { min, max };
}

template<typename T>
std::vector<T> fill_vector(size_t size) {
    std::vector<T> result(size);
    for (int i = 0; i < size; ++i) {
        result[i] = static_cast<T>(i);
    }
    std::random_shuffle(result.begin(), result.end());
    return result;
}

static void escape(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}

int main(int argc, char** argv) {
    size_t start_size = 4 * 1024;
    size_t end_size = 16 * 1024 * 1024;

    LIKWID_MARKER_INIT;

    for (size_t size = start_size; size <= end_size; size *= 2) {
        size_t repeat_count = 4 * end_size / size;
        std::vector<double> vec = fill_vector<double>(size);

        std::string separate = "separate_" + std::to_string(size);
        std::string merged = "merged_" + std::to_string(size);

        LIKWID_MARKER_START(separate.c_str());
        for (int i = 0; i < repeat_count; i++) {
            double min = find_min<double>(vec.begin(), vec.end());
            double max = find_max<double>(vec.begin(), vec.end());
            escape(&min);
            escape(&max);
        }
        LIKWID_MARKER_STOP(separate.c_str());

        LIKWID_MARKER_START(merged.c_str());
        for (int i = 0; i < repeat_count; i++) {
            auto minmax = find_minmax<double>(vec.begin(), vec.end());
            escape(&minmax);
        }
        LIKWID_MARKER_STOP(merged.c_str());

    }

    LIKWID_MARKER_CLOSE;

}