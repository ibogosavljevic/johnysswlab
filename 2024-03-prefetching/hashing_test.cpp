
#include <unordered_set>
#include <iostream>
#include <random>
#include <algorithm>
#include "fast_hash_map.h"
#include "likwid.h"


template <typename T>
std::vector<T> create_random_array(int n, int min, int max);

template<>
std::vector<int> create_random_array(int n, int min, int max) {
  std::random_device r;
  std::seed_seq      seed{r(), r(), r(), r(), r(), r(), r(), r()};
  std::mt19937       eng(seed); // a source of random data

  std::uniform_int_distribution<int> dist(min, max - 1);
  std::vector<int> v(n);

  generate(begin(v), end(v), bind(dist, eng));
  return v;
}

template <typename Q, size_t look_ahead, bool prefetch>
size_t run_nanothreads_test(fast_hash_map<Q, hash_map_entry<Q>>& map, std::vector<int>& v, size_t size, size_t iterations, std::string& suffix) {
    size_t found = 0;

    std::string prefetch_str = (prefetch ? "_prefetch" : "_noprefetch");

    std::string name = "fast_hash_nano_" + std::to_string(look_ahead) + prefetch_str + suffix; 

    LIKWID_MARKER_START(name.c_str());
    
    for (size_t j = 0; j < iterations; j++) {
        std::vector<bool> result = map. template find_multiple_nanothreads<look_ahead, prefetch>(v);
        for (size_t i = 0; i < size; i++) {
            found += result[i];
        }
    }

    LIKWID_MARKER_STOP(name.c_str());
    return found;
}


template <typename Q>
size_t run_test(size_t size) {
    std::vector<Q> v = create_random_array<Q>(size, 0, size);
    fast_hash_map<Q, hash_map_entry<Q>> my_fast_map(size);
    std::unordered_set<Q> reference_map(size);
    constexpr size_t found_cnt = 22;
    size_t found[found_cnt];

    for (size_t i = 0; i < found_cnt; i++) {
        found[i] = 0;
    }

    size_t iterations = 64 * 1024 * 1024 / size;
    iterations = iterations == 0 ? 1 : iterations;

    std::cout << "Iterations: " << iterations << std::endl;

    size_t count = 0.7 * size;

    for (size_t i = 0; i < count; i++) {
        reference_map.insert(Q(v[i]));
        my_fast_map.insert(Q(v[i]));
    }

    for (size_t i = 0.3 * size; i < size; i++) {
        reference_map.erase(v[i]);
        my_fast_map.remove(v[i]);
    }

    std::string suffix = "_" + std::to_string(size);

    {
        std::string name = "stl" + suffix;
        LIKWID_MARKER_START(name.c_str());
        for (size_t j = 0; j < iterations; j++) {
            std::vector<bool> result(size, false);
            for (size_t i = 0; i < size; i++) {
                result[i] = reference_map.find(v[i]) != reference_map.end();
            }
            for (size_t i = 0; i < size; i++) {
                found[0] += result[i];
            }
        }
        LIKWID_MARKER_STOP(name.c_str());
    }

    {
        std::string name = "fast_hash_simple" + suffix;
        LIKWID_MARKER_START(name.c_str());
        for (size_t j = 0; j < iterations; j++) {
            std::vector<bool> result = my_fast_map.find_multiple_simple(v);
            for (size_t i = 0; i < size; i++) {
                found[1] += result[i];
            }
        }
        LIKWID_MARKER_STOP(name.c_str());
    }

    found[2] = run_nanothreads_test<Q, 1, false>(my_fast_map, v, size, iterations, suffix);
    found[3] = run_nanothreads_test<Q, 1, true>(my_fast_map, v, size, iterations, suffix);
    found[4] = run_nanothreads_test<Q, 8, false>(my_fast_map, v, size, iterations, suffix);
    found[5] = run_nanothreads_test<Q, 8, true>(my_fast_map, v, size, iterations, suffix);
    found[6] = run_nanothreads_test<Q, 16, false>(my_fast_map, v, size, iterations, suffix);
    found[7] = run_nanothreads_test<Q, 16, true>(my_fast_map, v, size, iterations, suffix);
    found[8] = run_nanothreads_test<Q, 32, false>(my_fast_map, v, size, iterations, suffix);
    found[9] = run_nanothreads_test<Q, 32, true>(my_fast_map, v, size, iterations, suffix);
    found[10] = run_nanothreads_test<Q, 48, false>(my_fast_map, v, size, iterations, suffix);
    found[11] = run_nanothreads_test<Q, 48, true>(my_fast_map, v, size, iterations, suffix);
    found[12] = run_nanothreads_test<Q, 64, false>(my_fast_map, v, size, iterations, suffix);
    found[13] = run_nanothreads_test<Q, 64, true>(my_fast_map, v, size, iterations, suffix);
    found[14] = run_nanothreads_test<Q, 96, false>(my_fast_map, v, size, iterations, suffix);
    found[15] = run_nanothreads_test<Q, 96, true>(my_fast_map, v, size, iterations, suffix);
    found[16] = run_nanothreads_test<Q, 128, false>(my_fast_map, v, size, iterations, suffix);
    found[17] = run_nanothreads_test<Q, 128, true>(my_fast_map, v, size, iterations, suffix);
    found[18] = run_nanothreads_test<Q, 192, false>(my_fast_map, v, size, iterations, suffix);
    found[19] = run_nanothreads_test<Q, 192, true>(my_fast_map, v, size, iterations, suffix);
    found[20] = run_nanothreads_test<Q, 256, false>(my_fast_map, v, size, iterations, suffix);
    found[21] = run_nanothreads_test<Q, 256, true>(my_fast_map, v, size, iterations, suffix);

    for (size_t i = 0; i < 22; i++) {
        assert(found[0] == found[i]);
    }


    std::cout << "Found0 = " << found[0] << ", found6 = " << found[6] << std::endl;

    return found[0];
}

int main(int argc, const char* argv[]) {
    size_t start_size = 4 * 1024;
    size_t end_size = 64 * 1024 * 1024;

    LIKWID_MARKER_INIT;

    for (size_t s = start_size; s <= end_size; s*=4) {
        size_t found = run_test<int>(s);
        std::cout << "Found " << found << std::endl;
    }

    LIKWID_MARKER_CLOSE;

    return 0;
}
