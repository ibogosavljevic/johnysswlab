
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
                size_t& out_element_size) {
    ArgumentParser parser("test123", "123");

    parser.add_argument("-h", "--hash",
                        "Type of the hash function (simple, shift)", true);
    parser.add_argument("-s", "--size",
                        "Size of the element in the container (4, 124, 128)",
                        false);

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
        int size = parser.get<int>("s");
        std::cout << "Size of the element in the container : " << size
                  << std::endl;
        if (size == 4 || size == 124 || size == 128) {
            out_element_size = size;
        } else {
            std::cout << "Unknown value for --size\n";
            return false;
        }
    } else {
        out_element_size = 4;
    }

    return true;
}

template <typename T, typename Q>
void run_test() {
    constexpr int arr_len = 50 * 1024 * 1024;
    std::vector<int> v = create_random_array<int>(arr_len, 0, arr_len);
    hash_map<int, T> my_map(arr_len);

    for (size_t i = 0; i < 42 * 1024 * 1024; i++) {
        my_map.insert(v[i]);
    }
}

int main(int argc, const char* argv[]) {
    hash_type_e hash_type;
    size_t size;

    if (!parse_args(argc, argv, hash_type, size)) {
        return false;
    }

    if (size == 4) {
        if (hash_type == hash_type_e::SIMPLE) {
            run_test<simple_hasher, int>();
        } else if (hash_type == hash_type_e::SHIFT) {
            run_test<shift_hasher, int>();
        }
    } else if (size == 124) {
        if (hash_type == hash_type_e::SIMPLE) {
            run_test<simple_hasher, int>();
        } else if (hash_type == hash_type_e::SHIFT) {
            run_test<shift_hasher, int>();
        }
    } else if (size == 128) {
        if (hash_type == hash_type_e::SIMPLE) {
            run_test<simple_hasher, int>();
        } else if (hash_type == hash_type_e::SHIFT) {
            run_test<shift_hasher, int>();
        }
    }
    return 0;
}
