#include <algorithm>
#include <vector>
#include <iostream>
#include <limits>
#include <cassert>
#include "function_runner.h"
#include "memcpy.h"

using namespace std;

int binary_search(int *array, int number_of_elements, int key)
{
    int low = 0, high = number_of_elements - 1, mid;
    while (low <= high)
    {
        mid = (low + high) / 2;

        if (array[mid] < key)
            low = mid + 1;
        else if (array[mid] > key)
            high = mid - 1;
        else
            return mid;
    }
    return -1;
}

#define OPAQUE(x)                \
    ({                           \
        decltype(x) __tmp = (x); \
        asm(""                   \
            : "+r"(__tmp));      \
        __tmp;                   \
    })

int binary_search_branchless(int *array, int number_of_elements, int key)
{
    int low = 0, high = number_of_elements - 1, mid;
    while (low <= high)
    {
        mid = (low + high) / 2;

        bool less = array[mid] < key;
        bool greater = array[mid] > key;
        OPAQUE(less);
        OPAQUE(greater);

        low = less ? mid + 1 : low;
        high = greater ? mid - 1 : high;

        if (array[mid] == key)
        {
            return mid;
        }
    }
    return -1;
}

#define AVOID_CACHE_RO(addr) __builtin_prefetch(addr, 0, 0)
#define AVOID_CACHE_RW(addr) __builtin_prefetch(addr, 1, 0)

int binary_search_branchless_nt(int *array, int number_of_elements, int key)
{
    int low = 0, high = number_of_elements - 1, mid;
    while (low <= high)
    {
        mid = (low + high) / 2;

        AVOID_CACHE_RO(&array[mid]);

        bool less = array[mid] < key;
        bool greater = array[mid] > key;
        OPAQUE(less);
        OPAQUE(greater);

        low = less ? mid + 1 : low;
        high = greater ? mid - 1 : high;

        if (array[mid] == key)
        {
            return mid;
        }
    }
    return -1;
}

static const size_t LLC_CASE_SIZE_BYTES = 6 * 1024 * 1024;

std::vector<int> generate_sorted_vector(size_t elems)
{
    std::vector<int> result;
    result.reserve(elems);

    int val = numeric_limits<int>::min();

    for (size_t i = 0; i < elems; ++i)
    {
        result.push_back(val);
        val += (rand() % 2) + 1;
    }

    return result;
}

std::vector<int> create_lookup_data(const std::vector<int> &sorted_vec)
{
    int min = sorted_vec[0];
    int max = sorted_vec[sorted_vec.size() - 1];
    std::vector<int> result;

    for (int i = min; i <= max; ++i)
    {
        result.push_back(i);
    }

    random_shuffle(result.begin(), result.end());
    return result;
}

void dry_run(size_t len) {
    for (int i = 0; i < len; ++i) {
        __asm__ (
            "nop\n;"
            "nop\n"
            : [len] "+&r"(len)
            :
            : "cc", "memory"
        );
    }
}

template <typename IMPORT_F, typename UNIMPORT_F>
void run_test(IMPORT_F f_import, UNIMPORT_F f_unimport, std::string description, bool run_unimport = true)
{
    function_runner f_important_runner;
    function_runner f_unimportant_runner;

    f_important_runner.run_async(f_import);
    if (run_unimport)
    {
        f_unimportant_runner.run_async(f_unimport);
    }
    this_thread::sleep_for(chrono::seconds(1));

    double import_exec_rate = f_important_runner.get_execution_rate();
    double unimport_exec_rate = f_unimportant_runner.get_execution_rate();

    f_important_runner.stop();
    if (run_unimport)
    {
        f_unimportant_runner.stop();
    }

    std::cout << description << ": IMPORTANT: " << import_exec_rate << " , UNIMPORT: " << unimport_exec_rate << "\n";
}

uint32_t *allocate_buffer(size_t elems)
{
    void *res = nullptr;
    posix_memalign(&res, 128, elems * sizeof(uint32_t));
    return (uint32_t *)res;
}

void free_buffer(uint32_t *buffer)
{
    free(buffer);
}

void check_eq(uint32_t* a, uint32_t* b, size_t n) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            std::cout << "NOT SAME\n";
            return;
        }
    }
    std::cout << "SAME\n";
}

void fill_buffer(uint32_t *buffer, size_t elems)
{
    srand(time(NULL));
    for (int i = 0; i < elems; ++i)
    {
        buffer[i] = rand();
    }
}

size_t round_to_16(uint32_t val) {
    return (val / 16) * 16;
}

void test_memcpy_implementations(size_t elems) {
    uint32_t* src = allocate_buffer(elems);
    uint32_t* dst = allocate_buffer(elems);

    fill_buffer(src, elems);

    fill_buffer(dst, elems);
    memcpy_simple(dst, src, elems);
    check_eq(src, dst, elems);

    fill_buffer(dst, elems);
    memcpy_fake_dep(dst, src, elems);
    check_eq(src, dst, elems);

    fill_buffer(dst, elems);
    memcpy_fence(dst, src, elems);
    check_eq(src, dst, elems);

    fill_buffer(dst, elems);
    memcpy_streaming_stores(dst, src, elems);
    check_eq(src, dst, elems);

    fill_buffer(dst, elems);
    memcpy_nt_loads(dst, src, elems);
    check_eq(src, dst, elems);

    fill_buffer(dst, elems);
    memcpy_streaming_stores_nt_loads(dst, src, elems);
    check_eq(src, dst, elems);

    fill_buffer(dst, elems);
    memcpy_streaming_stores_nt_loads_fence(dst, src, elems);
    check_eq(src, dst, elems);

    free(src);
    free(dst);
}

int main(int argc, char **argv)
{
    test_memcpy_implementations(4*1024);

    std::cout << "Dataset size is " << LLC_CASE_SIZE_BYTES / 1024 / 1024 << " MB. Please set the dataset size to be equal to the size of LLC on your system.\n";

    cout.setf(ios::fixed, ios::floatfield);
    cout.setf(ios::showpoint);

    size_t start_size = LLC_CASE_SIZE_BYTES / 4;
    size_t end_size = LLC_CASE_SIZE_BYTES * 4;

    for (size_t data_set_size_bytes = start_size; data_set_size_bytes <= end_size; data_set_size_bytes *= 2)
    {
        size_t data_set_size_elems = round_to_16(data_set_size_bytes / sizeof(int) - 6000); // -6000 to avoid cache conflicts

        std::cout << "Size of vector = " << data_set_size_elems * sizeof(int) / (1024.0 * 1024.0) << " megabytes\n";

        std::vector<int> sorted_data1 = generate_sorted_vector(data_set_size_elems);
        std::vector<int> lookup_data1 = create_lookup_data(sorted_data1);
        std::vector<int> sorted_data2 = generate_sorted_vector(data_set_size_elems);
        std::vector<int> lookup_data2 = create_lookup_data(sorted_data2);

        int imp_i = 0;
        int imp_total_found = 0;
        auto f_important_func = [&]() -> void
        {
            imp_total_found += binary_search(&sorted_data1[0], sorted_data1.size(), lookup_data1[imp_i]) != -1;
            imp_i++;
            if (imp_i >= lookup_data1.size())
            {
                imp_i = 0;
            }
        };

        run_test(f_important_func, f_important_func, "BINARY SEARCH ONLY", false);
        OPAQUE(imp_total_found);

        auto f_unimp_dummy_run = [&]() -> void {
            dry_run(data_set_size_elems);
        };
        run_test(f_important_func, f_unimp_dummy_run, "BINARY SEARCH + DUMMY RUN", true);

        int unimp_i = 0;
        int unimp_total_found = 0;
        auto f_unimport_binary_search = [&]() -> void
        {
            unimp_total_found += binary_search(&sorted_data2[0], sorted_data2.size(), lookup_data2[unimp_i]) != -1;
            unimp_i++;
            if (unimp_i >= lookup_data2.size())
            {
                unimp_i = 0;
            }
        };

        run_test(f_important_func, f_unimport_binary_search, "BINARY SEARCH + BINARY SEARCH", true);
        OPAQUE(unimp_total_found);
        OPAQUE(imp_total_found);

        auto f_unimport_binary_search_branchless = [&]() -> void
        {
            unimp_total_found += binary_search_branchless(&sorted_data2[0], sorted_data2.size(), lookup_data2[unimp_i]) != -1;
            unimp_i++;
            if (unimp_i >= lookup_data2.size())
            {
                unimp_i = 0;
            }
        };

        run_test(f_important_func, f_unimport_binary_search_branchless, "BINARY SEARCH + BINARY SEARCH BRANCHLESS", true);
        OPAQUE(unimp_total_found);
        OPAQUE(imp_total_found);

        auto f_unimport_binary_search_branchless_nt = [&]() -> void
        {
            unimp_total_found += binary_search_branchless_nt(&sorted_data2[0], sorted_data2.size(), lookup_data2[unimp_i]) != -1;
            unimp_i++;
            if (unimp_i >= lookup_data2.size())
            {
                unimp_i = 0;
            }
        };

        run_test(f_important_func, f_unimport_binary_search_branchless_nt, "BINARY SEARCH + BINARY SEARCH NON-TEMPORAL", true);
        OPAQUE(unimp_total_found);
        OPAQUE(imp_total_found);

        uint32_t* src1 = allocate_buffer(data_set_size_elems);
        uint32_t* src2 = allocate_buffer(data_set_size_elems);
        uint32_t* dst = allocate_buffer(data_set_size_elems);
        fill_buffer(src1, data_set_size_elems);
        fill_buffer(src2, data_set_size_elems);
        fill_buffer(dst, data_set_size_elems);

        auto f_unimport_memcpy_simple = [&]() -> void {
            memcpy_simple(dst, src1, data_set_size_elems);
            OPAQUE(dst);
            std::swap(src1, src2);
        };
        run_test(f_important_func, f_unimport_memcpy_simple, "BINARY SEARCH + MEMCPY SIMPLE", true);

        auto f_unimport_memcpy_fake_dep = [&]() -> void {
            memcpy_fake_dep(dst, src1, data_set_size_elems);
            OPAQUE(dst);
            std::swap(src1, src2);
        };
        run_test(f_important_func, f_unimport_memcpy_fake_dep, "BINARY SEARCH + MEMCPY FAKE DEP", true);

        auto f_unimport_memcpy_fence = [&]() -> void {
            memcpy_fence(dst, src1, data_set_size_elems);
            OPAQUE(dst);
            std::swap(src1, src2);
        };
        run_test(f_important_func, f_unimport_memcpy_fence, "BINARY SEARCH + MEMCPY FENCE", true);

        auto f_unimport_memcpy_streaming_stores = [&]() -> void {
            memcpy_streaming_stores(dst, src1, data_set_size_elems);
            OPAQUE(dst);
            std::swap(src1, src2);
        };
        run_test(f_important_func, f_unimport_memcpy_streaming_stores, "BINARY SEARCH + MEMCPY STREAMING STORES", true);

        auto f_unimport_memcpy_nt_loads = [&]() -> void {
            memcpy_nt_loads(dst, src1, data_set_size_elems);
            OPAQUE(dst);
            std::swap(src1, src2);
        };
        run_test(f_important_func, f_unimport_memcpy_nt_loads, "BINARY SEARCH + MEMCPY NT LOADS", true);

        auto f_unimport_memcpy_streaming_stores_nt_loads = [&]() -> void {
            memcpy_streaming_stores_nt_loads(dst, src1, data_set_size_elems);
            OPAQUE(dst);
            std::swap(src1, src2);
        };
        run_test(f_important_func, f_unimport_memcpy_streaming_stores_nt_loads, "BINARY SEARCH + MEMCPY STRAMING STORES NT LOADS", true);

        auto f_unimport_memcpy_streaming_stores_nt_loads_fence = [&]() -> void {
            memcpy_streaming_stores_nt_loads_fence(dst, src1, data_set_size_elems);
            OPAQUE(dst);
            std::swap(src1, src2);
        };
        run_test(f_important_func, f_unimport_memcpy_streaming_stores_nt_loads_fence, "BINARY SEARCH + MEMCPY STRAMING STORES NT LOADS FENCE", true);


        free(src1);
        free(src2);
        free(dst);
    }
}