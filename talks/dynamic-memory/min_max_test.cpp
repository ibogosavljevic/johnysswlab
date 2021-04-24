#include <algorithm>
#include "measure_time.h"
#include "utils.h"

struct test_class {
    int m_num;
    int m_unused[3];

    test_class(int num) : m_num(num) {}
};

static constexpr int ARR_LEN = 100 * 1024 * 1024;

int main(int argc, char** argv) {
    std::vector<test_class> v;
    v.reserve(ARR_LEN);

    for (int i = 0; i < ARR_LEN; i++) {
        v.emplace_back(i);
    }

    {
        measure_time m("split");

        int min = 1024 * 1024 * 1024, max = -1;
        for (int i = 0; i < ARR_LEN; i++) {
            min = std::min(min, v[i].m_num);
        }

        for (int i = 0; i < ARR_LEN; i++) {
            max = std::max(max, v[i].m_num);
        }

        std::cout << "Min = " << min << ", max = " << max << std::endl;
    }

    {
        measure_time m("merged");

        int min = 1024 * 1024 * 1024, max = -1;
        for (int i = 0; i < ARR_LEN; i++) {
            min = std::min(min, v[i].m_num);
            max = std::max(max, v[i].m_num);
        }

        std::cout << "Min = " << min << ", max = " << max << std::endl;
    }

    return 0;
}