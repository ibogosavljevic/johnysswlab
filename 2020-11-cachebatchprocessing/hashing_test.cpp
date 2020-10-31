
#include <unordered_set>
#include "common/argparse.h"
#include "fast_hash_map.h"
#include "measure_time.h"
#include "utils.h"

using namespace argparse;

bool parse_args(int argc, const char* argv[], size_t& out_size) {
    ArgumentParser parser("test123", "123");

    parser.add_argument("-h", "--hash",
                        "Type of the hash function (simple, shift)", true);
    parser.add_argument("-s", "--size", "Size of the hash map (s, m, l)", true);

    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
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

template <typename Q>
size_t run_test(int size) {
    int arr_len = size;
    std::vector<int> v = create_random_array<int>(arr_len, 0, arr_len);
    fast_hash_map<Q, simple_hash_map_entry<Q>> my_map2(arr_len);
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
    iterations = 1;

    int count = 0.7 * size;

    for (int j = 0; j < iterations; j++) {
        for (size_t i = 0; i < count; i++) {
            // my_map.insert(Q(v[i]));
            my_map2.insert(Q(v[i]));
            reference_map.insert(Q(v[i]));
        }

        // my_map.dump();
        // my_map2.dump(std::cout);

        for (size_t i = 0.3 * size; i < size; i++) {
            // my_map.remove(v[i]);
            my_map2.remove(v[i]);
            reference_map.erase(v[i]);
        }

        {
            measure_time m("STL");
            std::vector<bool> result(size, false);
            for (size_t i = 0; i < size; i++) {
                result[i] = reference_map.find(v[i]) != reference_map.end();
            }
            for (size_t i = 0; i < size; i++) {
                found0 += result[i];
            }
        }
        /*{
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
        }*/
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
        {
            measure_time m("fast_hash_map_multiple2");
            std::vector<bool> result = my_map2.find_multiple_fast2(v);
            for (size_t i = 0; i < size; i++) {
                found6 += result[i];
            }
        }
    }

    std::cout << "Found0 = " << found0 << ", found6 = " << found6 << std::endl;

    assert(found0 == found4);
    /*assert(found1 == found2);
    assert(found1 == found3);
    assert(found1 == found4);*/
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
