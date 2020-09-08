
#include <unordered_set>
#include "common/argparse.h"
#include "hash_map.h"
#include "utils.h"

using namespace jsl;
using namespace argparse;

enum hash_type_e { SIMPLE, SHIFT };

bool parse_args(int argc, const char* argv[], hash_type_e& out_hash_type) {
    ArgumentParser parser("test123", "123");

    parser.add_argument("-h", "--hash",
                        "Type of the hash function (simple, shift)", true);

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

    return true;
}

template <typename T>
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

    if (!parse_args(argc, argv, hash_type)) {
        return false;
    }

    if (hash_type == hash_type_e::SIMPLE) {
        run_test<simple_hasher>();
    } else if (hash_type == hash_type_e::SHIFT) {
        run_test<shift_hasher>();
    }
    return 0;
}
