#include <vector>
#include "likwid.h"
#include "omp.h"

#include <cassert>
#include <string>
#include <algorithm>
#include <numeric>
#include <iostream>

static void clobber() {
  asm volatile("" : : : "memory");
}

__attribute__((noinline)) int run_test(std::vector<int>& data, std::vector<int>& indexes, std::string test_name, int repeat_count, int num_cores) {
    assert(data.size() == indexes.size());
    int len = data.size();
    int result;

    std::string name = test_name + "_" + std::to_string(len) + "_" + std::to_string(num_cores) + "_" + std::to_string(repeat_count);

    int sum = 0;
    #pragma omp parallel num_threads(num_cores) default(none) shared(indexes, data, repeat_count, len, name, sum)
    {
        std::vector<int> data_priv = data;
        std::vector<int> index_priv = indexes;

        int* data_ptr = &data_priv[0];
        int* indexes_ptr = &index_priv[0];

        int sum_priv = 0;

        LIKWID_MARKER_START(name.c_str());

        for (int r = 0; r < repeat_count; r++) {
            for (int i = 0; i < len; i++) {
                sum_priv += data_ptr[indexes_ptr[i]];
            }
            clobber();
        }
        LIKWID_MARKER_STOP(name.c_str());
        
        #pragma omp critical
        sum += sum_priv;
    }

    result = sum;

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
    LIKWID_MARKER_THREADINIT;

    int start_size = 4*1024;
    int end_size = 64*1024*1024;

    int max_core_count = omp_get_max_threads();
    if (argc >= 2) {
        max_core_count = std::atoi(argv[1]);
    }
    std::cout << "Max cores = " << max_core_count << std::endl;

    for (int cores = 1; cores <= max_core_count; ++cores) {
        std::cout << "Running with " << cores << " cores\n";
        for (int size = start_size; size <= end_size; size *= 2) {
            std::cout << "Size = " << size/1024 << "KB\n";
            int repeat_count = end_size / size * 8;

            std::vector<int> data(size);
            
            std::iota(data.begin(), data.end(), 0);
            std::random_shuffle(data.begin(), data.end());

            std::vector<int> indexes(size);
            std::iota(indexes.begin(), indexes.end(), 0);
            int sum1 = run_test(data, indexes, "Sequential", repeat_count, cores);
            std::cout << sum1 << std::endl;

            std::random_shuffle(indexes.begin(), indexes.end());
            int sum2 = run_test(data, indexes, "Random", repeat_count, cores);
            std::cout << sum2 << std::endl;

            generate_random_index_vector(indexes.begin(), indexes.end(), size, 4);
            int sum3 = run_test(data, indexes, "Stride_4", repeat_count, cores);
            std::cout << sum3 << std::endl;

            generate_random_index_vector(indexes.begin(), indexes.end(), size, 16);
            int sum4 = run_test(data, indexes, "Stride_16", repeat_count, cores);
            std::cout << sum4 << std::endl;

            generate_random_index_vector(indexes.begin(), indexes.end(), size, 32);
            int sum5 = run_test(data, indexes, "Stride_32", repeat_count, cores);
            std::cout << sum5 << std::endl;

            generate_random_index_vector(indexes.begin(), indexes.end(), size, 64);
            int sum6 = run_test(data, indexes, "Stride_64", repeat_count, cores);
            std::cout << sum6 << std::endl;

            generate_random_index_vector(indexes.begin(), indexes.end(), size, 128);
            int sum7 = run_test(data, indexes, "Stride_128", repeat_count, cores);
            std::cout << sum7 << std::endl;
        }
    }

    LIKWID_MARKER_CLOSE;


    return 0;
}
