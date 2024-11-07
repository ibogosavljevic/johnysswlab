#include "../common/argparse.h"
#include "likwid.h"

#include <limits>
#include <cmath>

template <typename T>
void assert_buffers_equal(T const * const buff0, T const * const buff1, size_t const size) {
    for (size_t i = 0; i < size; ++i) {
        if (buff0[i] != buff1[i]) {
            std::cout << "Buffers not equal at position " << i << std::endl;
            return;
        }
    }
    std::cout << "Buffers equal\n";
}

size_t binary_search(uint32_t* array, size_t size, uint32_t key) {
    int64_t low = 0, high = size - 1, mid;
    while(low <= high) {
        mid = (low + high)/2;

        if(array[mid] < key)
            low = mid + 1; 
        else if(array[mid] > key)
             high = mid-1;
        else if(array[mid] == key)
             return mid;
    }
    return std::numeric_limits<size_t>::max();
}

void partial_sort(uint32_t* out, uint32_t const* array, size_t size, size_t b, uint32_t max_size) {
    size_t buckets = 1 << b;
    std::vector<std::vector<uint32_t>> helper_array(buckets);

    uint32_t values_in_bucket = max_size / buckets;

    for (size_t i = 0; i < size; i++) {
        uint32_t idx = array[i] / values_in_bucket;
        if (idx >= buckets) {
            idx = buckets - 1;
        }

        helper_array[idx].push_back(array[i]);
    }

    uint32_t j = 0;

    for (size_t idx = 0; idx < buckets; idx++) {
        for (size_t i = 0; i < helper_array[idx].size(); i++) {
            out[j] = helper_array[idx][i];
            j++;
        }
    }
}

void verify_results(const std::vector<uint32_t>& sorted, const std::vector<uint32_t>& lookup_values, const std::vector<size_t>& res) {
    for (size_t i = 0; i < lookup_values.size(); i++) {
        if (res[i] != std::numeric_limits<size_t>::max()) {
            if (lookup_values[i] != sorted[res[i]]) {
                std::cout << "Result array wrong at position " << i << std::endl;
                std::cout << "\tLookup values " << lookup_values[i] << std::endl;
                std::cout << "\tIdx " << res[i] << std::endl;
                std::cout << "\tSorted[idx] " << sorted[res[i]] << std::endl;
                return;
            }
        }
    }
    std::cout << "Result array correct\n";
}

static void clobber() {
    asm volatile("" : : : "memory");
}

using namespace argparse;

int main(int argc, const char* argv[]) {
    ArgumentParser parser("matrix_rotate", "matrix_rotate");

    parser.add_argument("-s", "--size", "Sorted array size", true);
    parser.add_argument("-v", "--values", "Number of lookup values", true);
    parser.add_argument("-b", "--buckets", "Number of partitioning buckets. The actual number is 2^b", true);

    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return false;
    }
    
    size_t sorted_size = 0;
    size_t lookup_values_count = 0;
    size_t b = 0;

    if (parser.exists("s")) {
        sorted_size = parser.get<size_t>("s");
    }

    if (parser.exists("v")) {
        lookup_values_count = parser.get<size_t>("v");
    }

    if (parser.exists("buckets")) {
        b = parser.get<size_t>("b");
        if (b > 32) {
            b = 32;
        }

        while ((1U << b) > lookup_values_count) {
            b--;
        }
    }

    size_t const buckets = (1ULL << b); 
    size_t const repeat_count = std::max(1UL, 128UL * 1024UL * 1024UL / (lookup_values_count * static_cast<size_t>(log2(sorted_size))));


    std::cout << "Sorted array size " << sorted_size << ", total lookup values " << lookup_values_count << ", total buckets " << buckets << "\n";
    std::cout << "Repeat count " << repeat_count << "\n";

    std::vector<uint32_t> v(std::max(sorted_size, lookup_values_count) * 2);
    std::iota(v.begin(), v.end(), 0U);

    std::random_shuffle(v.begin(), v.end());
    std::vector<uint32_t> sorted(v.begin(), v.begin() + sorted_size);
    std::sort(sorted.begin(), sorted.end());

    std::random_shuffle(v.begin(), v.end());
    std::vector<uint32_t> lookup_values(v.begin(), v.begin() + lookup_values_count);

    std::vector<size_t> res0(lookup_values_count);
    std::vector<size_t> res1(lookup_values_count);

    LIKWID_MARKER_INIT;

    // Warmup
    for (size_t i = 0; i < lookup_values_count; i++) {
        res0[i] = binary_search(sorted.data(), sorted_size, lookup_values[i]);
        res1[i] = res0[i];
    }

    LIKWID_MARKER_START("original");
    for (size_t r = 0; r < repeat_count; r++) {
        for (size_t i = 0; i < lookup_values_count; i++) {
            res0[i] = binary_search(sorted.data(), sorted_size, lookup_values[i]);
        }
        clobber();
    }
    LIKWID_MARKER_STOP("original");

    verify_results(sorted, lookup_values, res0);

    std::vector<uint32_t> lookup_values2(lookup_values.size());

    LIKWID_MARKER_START("partial_sorting");
    for (size_t r = 0; r < repeat_count; r++) {
        partial_sort(lookup_values2.data(), lookup_values.data(), lookup_values_count, b, std::max(sorted_size, lookup_values_count) * 2 + 1);
        for (size_t i = 0; i < lookup_values_count; i++) {
            res1[i] = binary_search(sorted.data(), sorted_size, lookup_values2[i]);
        }
        clobber();
    }
    LIKWID_MARKER_STOP("partial_sorting");

    verify_results(sorted, lookup_values2, res1);

    std::sort(lookup_values.begin(), lookup_values.end());
    std::sort(lookup_values2.begin(), lookup_values2.end());

    assert_buffers_equal(lookup_values.data(), lookup_values2.data(), lookup_values2.size());

    LIKWID_MARKER_CLOSE;
}