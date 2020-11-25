
#include <iostream>
#include "measure_time.h"
#include "utils.h"

int main(int argc, char* argv[]) {
    constexpr int size = 100 * 1024 * 1024;
    std::vector<int> input = create_random_array<int>(size, 0, size);

    std::vector<int> map(size);

    // Generates sequence 0, 1, 2, etc
    std::generate(map.begin(), map.end(), [n = 0]() mutable { return n++; });

    // Generates sequence n, n - 1, n - 2, etc
    // std::generate(map.begin(), map.end(),
    //              [n = size - 1]() mutable { return n--; });

    // Generates sequence 0, 16, 24, ..., etc
    /* std::generate(map.begin(), map.end(), [size, n = 0]() mutable {
        n = n < size ? n + 16 : n % size;
        return n;
    }); */

    // Random shuffle of the array
    // std::random_shuffle(map.begin(), map.end());

    int index;
    int sum = 0;
    {
        measure_time m("");
        for (int i = 0; i < size; i++) {
            index = map[i];
            sum += input[index];
        }
    }

    std::cout << "sum = " << sum << std::endl;
}
