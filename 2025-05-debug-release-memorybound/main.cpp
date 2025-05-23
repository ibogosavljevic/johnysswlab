
#include <vector>
#include <random>
#include <cstdint>
#include <algorithm>

#include "likwid.h"

static void escape(void* p) {
  asm volatile ("" : : "g"(p) : "memory");
}

std::vector<int64_t> generate_random_data(std::size_t size) {
    static std::mt19937_64 rng(std::random_device{}());
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

int64_t run_test(size_t input_size, size_t pointer_size) {
    int64_t sum { 0ULL };
    size_t p { 0ULL };

    std::vector<int64_t> vector = generate_random_data(input_size);
    std::vector<size_t> pointers = generate_random_pointers(input_size, pointer_size);

    std::string name = "test_" + std::to_string(input_size);

    LIKWID_MARKER_START(name.c_str());

    for (size_t i { 0ULL }; i < pointers.size(); i++) {
        sum += vector[pointers[i]];
    }

    LIKWID_MARKER_STOP(name.c_str());

    escape(&sum);

    return sum;
}

int main(int argc, char * argv[]) {
    static constexpr size_t total_lookups { 32ULL * 1024ULL * 1024ULL };

    LIKWID_MARKER_INIT;

    run_test(1000ULL, total_lookups);
    run_test(8000ULL, total_lookups);
    run_test(64000ULL, total_lookups);
    run_test(512000ULL, total_lookups);
    run_test(4096000ULL, total_lookups);
    run_test(32768000ULL, total_lookups);
    run_test(262144000ULL, total_lookups);
    run_test(662144000ULL, total_lookups);

    LIKWID_MARKER_CLOSE;

    return 0;
}