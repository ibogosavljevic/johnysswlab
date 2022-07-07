#include <vector>
#include "likwid.h"

#include <cassert>
#include <string>
#include <algorithm>
#include <numeric>
#include <iostream>

static constexpr int TEST_SIZE = 64 * 1024 * 1024;

__attribute__((noinline)) int run_test(std::vector<int>& data, std::vector<int>& indexes, std::string name) {
    assert(data.size() == indexes.size());
    int len = data.size();
    int* data_ptr = &data[0];
    int* indexes_ptr = &indexes[0];
    int result;

    LIKWID_MARKER_START(name.c_str());

    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += data_ptr[indexes_ptr[i]];
    }

    result = sum;

    LIKWID_MARKER_STOP(name.c_str());

    /*double time = 0.0;
    int count = 0;
    int data_volume = (4 * 2 * len);
    double data_volume_mb = data_volume / (1024 * 1024);

    std::cout << "Region " << name << "\n";
    std::cout << "Calculated data volume = " << data_vol    ume / (1024 * 1024) << " MB\n";
    LIKWID_MARKER_GET(name.c_str(), nullptr, nullptr, &time, &count);
    std::cout << "Calculated throughput = " << data_volume_mb / time << " MB/s\n";*/

    return result;
}

template <typename T>
void generate_random_index_vector(T begin, T end, int size, int stride) {
    int current = 0;
    int start = 0;

    for (auto it = begin; it != end; ++it) {
        *it = current;
        current += stride;
        if (current >= size) {
            start++;
            current = start;
        }
    }
}

int main(int argc, char** argv) {

    LIKWID_MARKER_INIT;

    std::vector<int> data(TEST_SIZE);
    
    std::iota(data.begin(), data.end(), 0);
    std::random_shuffle(data.begin(), data.end());

    std::vector<int> indexes(TEST_SIZE);
    std::iota(indexes.begin(), indexes.end(), 0);
    int sum1 = run_test(data, indexes, "Sequential");
    std::cout << sum1 << std::endl;

    std::random_shuffle(indexes.begin(), indexes.end());
    int sum2 = run_test(data, indexes, "Random");
    std::cout << sum2 << std::endl;

    generate_random_index_vector(indexes.begin(), indexes.end(), TEST_SIZE, 4);
    int sum3 = run_test(data, indexes, "Stride_4");
    std::cout << sum3 << std::endl;

    generate_random_index_vector(indexes.begin(), indexes.end(), TEST_SIZE, 16);
    int sum4 = run_test(data, indexes, "Stride_16");
    std::cout << sum4 << std::endl;

    generate_random_index_vector(indexes.begin(), indexes.end(), TEST_SIZE, 32);
    int sum5 = run_test(data, indexes, "Stride_32");
    std::cout << sum5 << std::endl;

    generate_random_index_vector(indexes.begin(), indexes.end(), TEST_SIZE, 64);
    int sum6 = run_test(data, indexes, "Stride_64");
    std::cout << sum6 << std::endl;

    generate_random_index_vector(indexes.begin(), indexes.end(), TEST_SIZE, 128);
    int sum7 = run_test(data, indexes, "Stride_128");
    std::cout << sum7 << std::endl;

    LIKWID_MARKER_CLOSE;


    return 0;
}
