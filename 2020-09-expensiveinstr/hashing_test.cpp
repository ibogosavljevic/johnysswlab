
#include <unordered_set>
#include "common/argparse.h"
#include "hash_map.h"
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
            out_size = 16 * 1024;
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
    size_t found = 0;

    size_t iterations = 64 * 1024 * 1024 / size;
    iterations = iterations == 0 ? 1 : iterations;

    int count = 0.7 * size;

    for (int j = 0; j < iterations; j++) {
        for (size_t i = 0; i < count; i++) {
            my_map.insert(Q(v[i]));
        }

        for (size_t i = 0.3 * size; i < size; i++) {
            my_map.remove(v[i]);
        }

        for (size_t i = 0; i < size; i++) {
            found += my_map.find(v[i]);
        }
    }

    return found;
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
