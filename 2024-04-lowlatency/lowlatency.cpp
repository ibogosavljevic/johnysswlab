#include <unordered_set>
#include <random>
#include <limits>
#include <iostream>
#include <x86intrin.h>


enum TEST_TYPE {
    REGULAR,
    RELOAD_ZERO,
    RELOAD_RANDOM
};

void measure_latency(size_t hash_map_entries, size_t num_lookups)
{
    std::unordered_set<int> test_set;
    constexpr int64_t diff = 10000000;

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, hash_map_entries * 4);

    for (size_t i = 0; i < hash_map_entries; ++i) {
        test_set.insert(dist(rng));
    }

    int64_t lookup_regular_latency = 0;
    int64_t total_regular_found = 0;
    
    for (size_t i = 0; i < num_lookups; ++i) {
        int val = dist(rng);
        int64_t start = __rdtsc();
        int64_t current = __rdtsc();
        while((current - start) < diff) {
            _mm_pause();
            current = __rdtsc();
        }

        int64_t lookup_start = __rdtsc();
        total_regular_found += test_set.find(val) != test_set.end();
        int64_t lookup_end = __rdtsc();

        lookup_regular_latency += (lookup_end - lookup_start);
    }

    int64_t lookup_reload_zero_latency = 0;
    int64_t total_reload_zero_found = 0;
    int64_t zero_found = 0;
    
    for (size_t i = 0; i < num_lookups; ++i) {
        int val = dist(rng);
        int64_t start = __rdtsc();
        int64_t current = __rdtsc();
        while((current - start) < diff) {
            zero_found += test_set.find(0) != test_set.end();
            _mm_pause();
            current = __rdtsc();
        }

        int64_t lookup_start = __rdtsc();
        total_reload_zero_found += test_set.find(val) != test_set.end();
        int64_t lookup_end = __rdtsc();

        lookup_reload_zero_latency += (lookup_end - lookup_start);
    }

    int64_t lookup_reload_random_latency = 0;
    int64_t total_reload_random_found = 0;
    int64_t random_found = 0;

    for (size_t i = 0; i < num_lookups; ++i) {
        int val = dist(rng);
        int64_t start = __rdtsc();
        int64_t current = __rdtsc();
        while((current - start) < diff) {
            random_found += test_set.find(dist(rng)) != test_set.end();
            _mm_pause();
            current = __rdtsc();
        }

        int64_t lookup_start = __rdtsc();
        total_reload_random_found += test_set.find(val) != test_set.end();
        int64_t lookup_end = __rdtsc();

        lookup_reload_random_latency += (lookup_end - lookup_start);
    }

    std::cout << "Hash map size: " << hash_map_entries / 1024 << "K entries\n";

    std::cout << "Lookup latency\n";
    std::cout << "    Regular      : " << lookup_regular_latency << "\n";
    std::cout << "    Reload zero  : " << lookup_reload_zero_latency << "\n";
    std::cout << "    Reload random: " << lookup_reload_random_latency << "\n";

    std::cout << "Total regular found: " << total_regular_found << "\n";
    std::cout << "Total reload zero found: " << total_reload_zero_found << "\n";
    std::cout << "Total reload random found: " << total_reload_random_found << "\n";

    std::cout << "Zero found " << zero_found << "\n";
    std::cout << "Random found " << random_found << "\n";

}


int main(int argc, char* argv[])
{
    size_t start_size = 1024;
    size_t end_size = 64*1024*1024;

    for (size_t size = start_size; size <= end_size; size *= 4) {
        measure_latency(size, 10000);
    }
    
    return 0;
}