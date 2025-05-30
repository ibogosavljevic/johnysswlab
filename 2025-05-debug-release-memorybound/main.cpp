
#include <vector>
#include <random>
#include <cstdint>
#include <algorithm>

#include "likwid.h"

static void escape(void* p) {
  asm volatile ("" : : "g"(p) : "memory");
}

std::vector<int64_t> generate_random_data(std::size_t size) {
    std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<int64_t> dist(-100, 100);

    std::vector<int64_t> result(size);
    for (auto& val : result) {
        val = dist(rng);
    }
    return result;
}

std::vector<size_t> generate_random_pointers(size_t input_size, size_t pointer_size) {
    std::vector<size_t> pointers;
    pointers.reserve(pointer_size);
    size_t v { 0ULL };

    for (size_t i { 0ULL }; i < pointer_size; i++) {
        pointers.push_back(v);
        v++;
        if (v >= input_size) {
            v = 0ULL;
        }
    }

    std::shuffle(pointers.begin(), pointers.end(), std::mt19937{std::random_device{}()});

    return pointers;
}

int64_t run_test_high_ilp(size_t input_size, size_t pointer_size) {
    int64_t sum { 0ULL };
    size_t p { 0ULL };

    std::vector<int64_t> vector = generate_random_data(input_size);
    std::vector<size_t> pointers = generate_random_pointers(input_size, pointer_size);

    std::string name = "test_highilp_" + std::to_string(input_size);

    LIKWID_MARKER_START(name.c_str());

    for (size_t i { 0ULL }; i < pointers.size(); i++) {
        sum += vector[pointers[i]];
    }

    LIKWID_MARKER_STOP(name.c_str());

    escape(&sum);

    return sum;
}

struct node_t {
    int64_t value;
    uint32_t next;

    static constexpr uint32_t NULLPTR { 0xFFFFFFFF };
};

std::vector<node_t> generate_linked_list(size_t size) {
    std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<int64_t> dist(-100, 100);

    std::vector<uint32_t> pointers(size);
    pointers[0] = 0;

    std::iota(pointers.begin() + 1, pointers.end(), 1U);
    std::shuffle(pointers.begin(), pointers.end(), rng);

    std::vector<node_t> result(size);


    for (size_t i { 0ULL }; i < size-1; i++) {
        result[pointers[i]].next = pointers[i+1];
        result[pointers[i]].value = dist(rng);
    }

    result[pointers[size-1]] = { dist(rng), node_t::NULLPTR };
    return result;
}

int64_t run_test_low_ilp(size_t input_size, size_t total_accesses) {
    int64_t sum { 0ULL };
    size_t p { 0ULL };
    uint32_t idx { 0U };

    std::vector<node_t> list = generate_linked_list(input_size);

    std::string name = "test_lowilp_" + std::to_string(input_size);

    LIKWID_MARKER_START(name.c_str());

    while(p < total_accesses) {
        sum += list[idx].value;
        idx = list[idx].next;
        
        if (idx == node_t::NULLPTR) {
            idx = 0U;
        }

        p++;
    }

    LIKWID_MARKER_STOP(name.c_str());

    escape(&sum);

    return sum;
}


int main(int argc, char * argv[]) {
    static constexpr size_t total_lookups { 32ULL * 1024ULL * 1024ULL };

    LIKWID_MARKER_INIT;

    run_test_high_ilp(1000ULL, total_lookups);
    run_test_high_ilp(8000ULL, total_lookups);
    run_test_high_ilp(64000ULL, total_lookups);
    run_test_high_ilp(512000ULL, total_lookups);
    run_test_high_ilp(4096000ULL, total_lookups);
    run_test_high_ilp(32768000ULL, total_lookups);
    run_test_high_ilp(262144000ULL, total_lookups);
    run_test_high_ilp(662144000ULL, total_lookups);

    run_test_low_ilp(1000ULL, total_lookups);
    run_test_low_ilp(8000ULL, total_lookups);
    run_test_low_ilp(64000ULL, total_lookups);
    run_test_low_ilp(512000ULL, total_lookups);
    run_test_low_ilp(4096000ULL, total_lookups);
    run_test_low_ilp(32768000ULL, total_lookups);
    run_test_low_ilp(262144000ULL, total_lookups);
    run_test_low_ilp(662144000ULL, total_lookups);


    LIKWID_MARKER_CLOSE;

    return 0;
}