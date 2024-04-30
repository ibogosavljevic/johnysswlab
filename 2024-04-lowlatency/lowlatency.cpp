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

    double m2 = 0.0;
    int64_t lookup_regular_latency = 0;
    int64_t total_regular_found = 0;
    int64_t min_latency_regular = 1000000;
    int64_t max_latency_regular = 0;
    double mean_regular = 0.0;
    double variance_regular = 0.0;

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

        int64_t lookup_latency = (lookup_end - lookup_start);
        lookup_regular_latency += lookup_latency;
        min_latency_regular = std::min(min_latency_regular, lookup_latency);
        max_latency_regular = std::max(max_latency_regular, lookup_latency);

        double delta = static_cast<double>(lookup_latency) - mean_regular;
        mean_regular += delta / static_cast<double>(i + 1);
        m2 += delta * delta;
        variance_regular = m2 / (i + 1);
    }

    m2 = 0.0;
    int64_t lookup_reload_zero_latency = 0;
    int64_t total_reload_zero_found = 0;
    int64_t zero_found = 0;
    int64_t min_latency_reload_zero = 1000000;
    int64_t max_latency_reload_zero = 0;
    double mean_reload_zero = 0.0;
    double variance_reload_zero = 0.0;
    
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

        int64_t lookup_latency = (lookup_end - lookup_start);
        lookup_reload_zero_latency += lookup_latency;
        min_latency_reload_zero = std::min(min_latency_reload_zero, lookup_latency);
        max_latency_reload_zero = std::max(max_latency_reload_zero, lookup_latency);

        double delta = static_cast<double>(lookup_latency) - mean_reload_zero;
        mean_reload_zero += delta / static_cast<double>(i + 1);
        m2 += delta * delta;
        variance_reload_zero = m2 / (i + 1);
    }

    m2 = 0.0;
    int64_t lookup_reload_random_latency = 0;
    int64_t total_reload_random_found = 0;
    int64_t random_found = 0;
    int64_t min_latency_reload_random = 1000000;
    int64_t max_latency_reload_random = 0;
    double mean_reload_random = 0.0;
    double variance_reload_random = 0.0;

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

        int64_t lookup_latency = (lookup_end - lookup_start);
        lookup_reload_random_latency += lookup_latency;
        min_latency_reload_random = std::min(min_latency_reload_random, lookup_latency);
        max_latency_reload_random = std::max(max_latency_reload_random, lookup_latency);

        double delta = static_cast<double>(lookup_latency) - mean_reload_random;
        mean_reload_random += delta / static_cast<double>(i + 1);
        m2 += delta * delta;
        variance_reload_random = m2 / (i + 1);
    }

    std::cout << "Hash map size: " << hash_map_entries / 1024 << "K entries\n";

    std::cout << "Lookup latency\n";
    std::cout << "    Regular      : " << lookup_regular_latency / num_lookups << "\n";
    std::cout << "    Reload zero  : " << lookup_reload_zero_latency / num_lookups << "\n";
    std::cout << "    Reload random: " << lookup_reload_random_latency / num_lookups << "\n";

    std::cout << "    Regular min latency      : " << min_latency_regular << ", max latency: " << max_latency_regular << "\n";
    std::cout << "    Reload zero min latency  : " << min_latency_reload_zero << ", max latency: " << max_latency_reload_zero << "\n";
    std::cout << "    Reload random min latency: " << min_latency_reload_random << ", max latency: " << max_latency_reload_random << "\n";

    std::cout << "    Regular mean        : " << mean_regular << " , stddev: " << std::sqrt(variance_regular) << "\n";
    std::cout << "    Reload zero mean    : " << mean_reload_zero << " , stddev: " << std::sqrt(variance_reload_zero) << "\n";
    std::cout << "    Reload random mean  : " << mean_reload_random << " , stddev: " << std::sqrt(variance_reload_random) << "\n";

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