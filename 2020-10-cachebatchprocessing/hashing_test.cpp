
#include <unordered_set>
#include "common/argparse.h"
#include "fast_hash_map.h"
#include "hash_map.h"
#include "measure_time.h"
#include "utils.h"

using namespace jsl;
using namespace argparse;

enum hash_type_e { SIMPLE, SHIFT };

bool parse_args(int argc,
                const char* argv[],
                hash_type_e& out_hash_type,
                size_t& out_size) {
    ArgumentParser parser("test123", "123");

    parser.add_argument("-h", "--hash",
                        "Type of the hash function (simple, shift)", true);
    parser.add_argument("-s", "--size", "Size of the hash map (s, m, l)", true);

    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return false;
    }

    if (parser.exists("h")) {
        std::string hash_type = parser.get<std::string>("h");
        std::cout << "Hash type : " << hash_type << std::endl;
        if (hash_type == "simple") {
            out_hash_type = hash_type_e::SIMPLE;
        } else if (hash_type == "shift") {
            out_hash_type = hash_type_e::SHIFT;
        } else {
            std::cout << "Unknown value for --hash\n";
            return false;
        }
    } else {
        return false;
    }

    if (parser.exists("s")) {
        std::string size = parser.get<std::string>("s");
        std::cout << "Size of the hash map : " << size << std::endl;
        if (size == "s") {
            out_size = 32;
        } else if (size == "m") {
            out_size = 8 * 1024 * 1024;
        } else if (size == "l") {
            out_size = 64 * 1024 * 1024;
        } else {
            std::cout << "Unknown value for --size\n";
            return false;
        }
    } else {
        return false;
    }

    return true;
}

template <typename T, typename Q>
size_t run_test(int size) {
    int arr_len = size;
    std::vector<int> v = create_random_array<int>(arr_len, 0, arr_len);
    hash_map<Q, T> my_map(arr_len);
    fast_hash_map<Q> my_map2(arr_len);
    // std::unordered_set reference_map(arr_len);
    size_t found1 = 0;
    size_t found2 = 0;
    size_t found3 = 0;
    size_t found4 = 0;
    size_t found5 = 0;

    size_t iterations = 64 * 1024 * 1024 / size;
    // iterations = iterations == 0 ? 1 : iterations;
    iterations = 1;

    int count = 0.7 * size;

    for (int j = 0; j < iterations; j++) {
        for (size_t i = 0; i < count; i++) {
            my_map.insert(Q(v[i]));
            my_map2.insert(Q(v[i]));
        }

        for (size_t i = 0.3 * size; i < size; i++) {
            my_map.remove(v[i]);
            my_map2.remove(v[i]);
        }

        {
            measure_time m("common");
            std::vector<bool> result(size, false);
            for (size_t i = 0; i < size; i++) {
                result[i] = my_map.find(v[i]);
            }
            for (size_t i = 0; i < size; i++) {
                found1 += result[i];
            }
        }
        {
            measure_time m("parallel 4");
            std::vector<bool> result = my_map.find(v);
            for (size_t i = 0; i < size; i++) {
                found2 += result[i];
            }
        }
        {
            measure_time m("multiple");
            std::vector<bool> result = my_map.find_multiple(v);
            for (size_t i = 0; i < size; i++) {
                found3 += result[i];
            }
        }
        {
            measure_time m("fast hash map simple");
            std::vector<bool> result = my_map2.find_multiple(v);
            for (size_t i = 0; i < size; i++) {
                found4 += result[i];
            }
        }
        {
            measure_time m("fast_hash_map_multiple");
            std::vector<bool> result = my_map2.find_multiple_fast(v);
            for (size_t i = 0; i < size; i++) {
                found5 += result[i];
            }
        }
    }

    std::cout << "Found1 = " << found1 << ", found5 = " << found5 << std::endl;

    assert(found1 == found2);
    assert(found1 == found3);
    // assert(found1 == found4);
    assert(found1 == found5);

    return found1;
}

int main(int argc, const char* argv[]) {
    hash_type_e hash_type;
    size_t size = 0;
    size_t found;

    if (!parse_args(argc, argv, hash_type, size)) {
        return false;
    }

    if (hash_type == hash_type_e::SIMPLE) {
        found = run_test<simple_hasher, int>(size);
    } else if (hash_type == hash_type_e::SHIFT) {
        found = run_test<shift_hasher, int>(size);
    }

    std::cout << "Found " << found << std::endl;

    return 0;
}
