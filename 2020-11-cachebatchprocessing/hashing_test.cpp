
#include <unordered_set>
#include "common/argparse.h"
#include "fast_hash_map.h"
#include "measure_time.h"
#include "utils.h"

using namespace argparse;

bool parse_args(int argc, const char* argv[], size_t& out_size) {
    ArgumentParser parser("test123", "123");

    parser.add_argument("-s", "--size", "Size of the hash map (s, m, l)", true);

    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return false;
    }

    if (parser.exists("s")) {
        std::string size = parser.get<std::string>("s");
        if (size == "s") {
            out_size = 32;
        } else if (size == "m") {
            out_size = 1 * 1024 * 1024;
        } else if (size == "l") {
            out_size = 64 * 1024 * 1024;
        } else {
            std::cout << "Unknown value for --size\n";
            return false;
        }
        std::cout << "Size of the hash map : " << out_size << std::endl;
    } else {
        return false;
    }

    return true;
}

template <typename Q>
size_t run_test(int size) {
    int arr_len = size;
    std::vector<int> v = create_random_array<int>(arr_len, 0, arr_len);
    fast_hash_map<Q, simple_hash_map_entry<Q>> my_simple_map(arr_len);
    fast_hash_map<Q, hash_map_entry<Q>> my_fast_map(arr_len);
    std::unordered_set<Q> reference_map(arr_len);
    size_t found0 = 0;
    size_t found1 = 0;
    size_t found2 = 0;
    size_t found3 = 0;
    size_t found4 = 0;
    size_t found5 = 0;
    size_t found6 = 0;

    size_t iterations = 64 * 1024 * 1024 / size;
    iterations = iterations == 0 ? 1 : iterations;

    std::cout << "Iterations: " << iterations << std::endl;

    int count = 0.7 * size;

    for (size_t i = 0; i < count; i++) {
        my_simple_map.insert(Q(v[i]));
        reference_map.insert(Q(v[i]));
        my_fast_map.insert(Q(v[i]));
    }

    // my_map.dump();
    // my_map2.dump(std::cout);

    for (size_t i = 0.3 * size; i < size; i++) {
        my_simple_map.remove(v[i]);
        reference_map.erase(v[i]);
        my_fast_map.remove(v[i]);
    }

    {
        measure_time m("STL");
        for (size_t j = 0; j < iterations; j++) {
            std::vector<bool> result(size, false);
            for (size_t i = 0; i < size; i++) {
                result[i] = reference_map.find(v[i]) != reference_map.end();
            }
            for (size_t i = 0; i < size; i++) {
                found0 += result[i];
            }
        }
    }
    {
        measure_time m("simple_map: regular find");
        for (size_t j = 0; j < iterations; j++) {
            std::vector<bool> result = my_simple_map.find_multiple_simple(v);
            for (size_t i = 0; i < size; i++) {
                found1 += result[i];
            }
        }
    }
    {
        measure_time m("simple_map: batch nano threads");
        for (size_t j = 0; j < iterations; j++) {
            std::vector<bool> result =
                my_simple_map.find_multiple_nanothreads(v);
            for (size_t i = 0; i < size; i++) {
                found2 += result[i];
            }
        }
    }
    {
        measure_time m("simple_map: batch alternate");
        for (size_t j = 0; j < iterations; j++) {
            std::vector<bool> result = my_simple_map.find_multiple_alternate(v);
            for (size_t i = 0; i < size; i++) {
                found3 += result[i];
            }
        }
    }
    {
        measure_time m("fast_map: regular find");
        for (size_t j = 0; j < iterations; j++) {
            std::vector<bool> result = my_fast_map.find_multiple_simple(v);
            for (size_t i = 0; i < size; i++) {
                found4 += result[i];
            }
        }
    }
    {
        measure_time m("fast_map: batch nano threads");
        for (size_t j = 0; j < iterations; j++) {
            std::vector<bool> result = my_fast_map.find_multiple_nanothreads(v);
            for (size_t i = 0; i < size; i++) {
                found5 += result[i];
            }
        }
    }
    {
        measure_time m("fast_map: batch alternate");
        for (size_t j = 0; j < iterations; j++) {
            std::vector<bool> result = my_fast_map.find_multiple_alternate(v);
            for (size_t i = 0; i < size; i++) {
                found6 += result[i];
            }
        }
    }

    std::cout << "Found0 = " << found0 << ", found6 = " << found6 << std::endl;

    assert(found0 == found4);
    assert(found1 == found2);
    assert(found1 == found3);
    assert(found1 == found4);
    assert(found4 == found5);
    assert(found5 == found6);

    return found1;
}

int main(int argc, const char* argv[]) {
    size_t size = 0;
    size_t found;

    if (!parse_args(argc, argv, size)) {
        return false;
    }

    found = run_test<int>(size);

    std::cout << "Found " << found << std::endl;

    return 0;
}
