
#include "hash_map.h"
#include <unordered_set>
#include "utils.h"

using namespace jsl;

int main(int argc, char* argv[]) {
    constexpr int arr_len = 50*1024*1024;
    std::vector<int> v = create_random_array<int>(arr_len, 0, arr_len);
    hash_map<int, shift_hasher> my_map(arr_len);

    for (size_t i = 0; i < 42 * 1024 * 1024; i++) {
        my_map.insert(v[i]);
    }

    return 0;
}
