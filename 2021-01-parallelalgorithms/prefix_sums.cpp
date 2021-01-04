#include <cstring>
#include "measure_time.h"
#include "utils.h"

static constexpr int MB = 1024 * 1024;

int main(int argc, char** argv) {
    std::vector<int> v = create_random_array<int>(100 * MB, 0, 100 * MB);
    std::vector<int> result(100 * MB, 0);

    // std::vector<int> v = { 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1
    // }; std::vector<int> result(17);

    {
        measure_time m("Serial prefix sums");

        result[0] = v[0];
        for (int i = 1; i < v.size(); i++) {
            result[i] = result[i - 1] + v[i];
        }
    }

    int res = 0;
    for (int i = 0; i < result.size(); i++) {
        res += result[i];
    }

    std::cout << "Result = " << res << std::endl;
    std::memset(&result[0], 0, result.size() * sizeof(int));

    {
        measure_time m("Parallel prefix sums");
        for (int i = 0; i < v.size(); i++) {
            result[i] = v[i];
        }

        int block_size;
        const int result_size = result.size();
        for (block_size = 2; block_size <= result_size; block_size *= 2) {
            int diff = block_size / 2;
            for (int i = block_size - 1; i < result_size; i += block_size) {
                result[i] = result[i] + result[i - diff];
            }
        }

        if ((block_size / 2) == result_size) {
            block_size = result_size;
        }

        for (block_size /= 2; block_size >= 2; block_size /= 2) {
            int diff = block_size / 2;

            int start = (result_size / block_size) * block_size + diff - 1;

            if (start >= result.size()) {
                start -= block_size;
            }

            for (int i = start; i >= block_size; i -= block_size) {
                result[i] = result[i] + result[i - diff];
            }
        }
    }

    res = 0;
    for (int i = 0; i < result.size(); i++) {
        res += result[i];
    }

    std::cout << "Result = " << res << std::endl;

    return 0;
}