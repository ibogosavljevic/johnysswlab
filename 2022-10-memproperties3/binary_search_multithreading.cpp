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

template <typename T>
void quickpartition(T array[], int left, int right, int max_part_size) {
    if (left >= right) {
        return;
    }

    int index_left = left;
    int index_right = right;
    int pivot_index = (left + right) / 2;
    T pivot = array[pivot_index];

    while (index_left < index_right) {
        if (array[index_left] <= pivot) {
            index_left++;
            continue;
        }

        if (array[index_right] > pivot) {
            index_right--;
            continue;
        }

        std::swap(array[index_left], array[index_right]);

        if (index_left == pivot_index) {
            pivot_index = index_right;
        }
        if (index_right == pivot_index) {
            pivot_index = index_left;
        }
    }

    if (array[index_right] > pivot) {
        std::swap(array[pivot_index], array[index_right - 1]);
        pivot_index = index_right - 1;
    } else {
        std::swap(array[pivot_index], array[index_right]);
        pivot_index = index_right;
    }

    int left_partition_size = pivot_index - 1 - left;
    int right_partition_size = right - pivot_index - 1;

    if (left_partition_size > max_part_size) {
        quickpartition(array, left, pivot_index - 1, max_part_size);
    }
    
    if (right_partition_size > max_part_size) {
        quickpartition(array, pivot_index + 1, right, max_part_size);
    }
    
}

struct binary_search_result_t {
    int index;
    long memory_accesses;
};

binary_search_result_t binary_search_bracnhless_asm(int* array, int number_of_elements, int key) {
    int low = 0, high = number_of_elements-1, mid;
    long mem_accesses = 0;
    binary_search_result_t result;

    while(low <= high) {
        mid = (low + high)/2;

        int mid_value = array[mid];
        mem_accesses++;

        if (mid_value == key) {
            result.index = mid;
            result.memory_accesses = mem_accesses;
            return result;
        }

        int new_low = mid + 1;
        int new_high = mid - 1;

#if defined(__x86_64)
            __asm__ (
                "cmp %[mid_value], %[key];"
                "cmovae %[new_low], %[low];"
                "cmovb %[new_high], %[high];"
                : [low] "+&r"(low), [high] "+&r"(high)
                : [new_low] "r"(new_low), [new_high] "r"(new_high), [mid_value] "r"(mid_value), [key] "r"(key)
                : "cc"
            );
#elif defined(__arm64__)
            __asm__ (
                "cmp %[array_middle], %[key];"
                "csel %[low], %[new_low], %[low], le;"
                "csel %[high], %[new_high], %[high], gt;"
                : [low] "+&r"(low), [high] "+&r"(high)
                : [new_low] "r"(new_low), [new_high] "r"(new_high), [array_middle] "r"(mid_value), [key] "r"(key)
                : "cc"
            );
#else
            bool cond_less = mid_value < key;

            low = cond_less ? new_low: low;
            high = cond_less ? high: new_high;
#endif
    }
    result.index = -1;
    result.memory_accesses = mem_accesses;
    return result;
}

__attribute__((noinline)) binary_search_result_t run_test(std::vector<int>& sorted_data, std::vector<int>& lookup_data, std::string test_name, int num_cores) {
    int lookup_cnt = lookup_data.size();
    int sorted_cnt = sorted_data.size();
    int* sorted_data_ptr = &sorted_data[0];
    int* lookup_data_ptr = &lookup_data[0];
    binary_search_result_t result;

    std::string name = test_name + "_" + std::to_string(sorted_data.size()) + "_" + std::to_string(num_cores);

    LIKWID_MARKER_START(name.c_str());

    int sum = 0;
    long mem_accesses = 0;
    #pragma omp parallel for reduction(+:sum, mem_accesses) num_threads(num_cores) schedule(static)
    for (int i = 0; i < lookup_cnt; i++) {
        binary_search_result_t res = binary_search_bracnhless_asm(sorted_data_ptr, sorted_cnt, lookup_data_ptr[i]);
        mem_accesses += res.memory_accesses;
        sum += (res.index != -1);
    }

    LIKWID_MARKER_STOP(name.c_str());

    result.memory_accesses = mem_accesses;
    result.index = sum;

    return result;
}


void generate_data(int data_size, int lookup_size, std::vector<int>& sorted_data, std::vector<int>& lookup_data) {
    //assert(lookup_size % data_size == 0);

    sorted_data.resize(data_size * 2);
    std::iota(sorted_data.begin(), sorted_data.end(), 0);
    std::random_shuffle(sorted_data.begin(), sorted_data.end());
    sorted_data.resize(data_size);
    std::sort(sorted_data.begin(), sorted_data.end());

    lookup_data.resize(lookup_size);
    int cnt = 0;
    for (int i = 0; i < lookup_size; i++) {
        lookup_data[i] = cnt;
        cnt++;
        if (cnt >= (data_size * 2)) {
            cnt = 0;
        }
    }

    std::random_shuffle(lookup_data.begin(), lookup_data.end());
}

void print(const std::vector<int>& v) {
    for (const auto& i: v) {
        std::cout << i << ", ";
    }
    std::cout << "\n";
}

int main(int argc, char** argv) {

    LIKWID_MARKER_INIT;

    // std::vector<int> sorted;
    // std::vector<int> lookup;

    // generate_data(16, 64, sorted, lookup);

    // print(sorted);
    // print(lookup);
    // binary_search_result_t res = run_test(sorted, lookup, "BINARYSEARCH", 1);
    // std::cout << "cores = " << 1 << ", size = " << 0 << "K, " << "memory accesses = " 
    //             << res.memory_accesses << ", found = " << res.index << "\n";

    int start_size = 4*1024;
    int end_size = 32*1024*1024;
    int total_lookups = end_size * 4;

    int max_core_count = omp_get_max_threads();
    if (argc >= 2) {
        max_core_count = std::atoi(argv[1]);
    }
    std::cout << "Max cores = " << max_core_count << std::endl;

    for (int cores = 1; cores <= max_core_count; ++cores) {
        std::cout << "Running with " << cores << " cores\n";
        for (int s = start_size; s <= end_size; s *= 2) {
            int size = s - (s/69);
            binary_search_result_t res;
            std::vector<int> sorted_data, lookup_data;
            generate_data(size, total_lookups, sorted_data, lookup_data);

            res = run_test(sorted_data, lookup_data, "BINARYSEARCHNOPART", cores);
            std::cout << "NOPART, cores = " << cores << ", size = " << size/1024 << "K, " << "memory accesses = " 
                      << res.memory_accesses << ", found = " << res.index << "\n";

            quickpartition(&lookup_data[0], 0, lookup_data.size() - 1, lookup_data.size()/(2 * cores));

            res = run_test(sorted_data, lookup_data, "BINARYSEARCHPART", cores);
            std::cout << "YEPART, cores = " << cores << ", size = " << size/1024 << "K, " << "memory accesses = " 
                      << res.memory_accesses << ", found = " << res.index << "\n";
        }
    }

    LIKWID_MARKER_CLOSE;


    return 0;
}
