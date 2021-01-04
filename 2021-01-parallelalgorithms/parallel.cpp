#include <omp.h>
#include <unordered_set>
#include "measure_time.h"
#include "utils.h"

static constexpr int MB = 1024 * 1024;

int main(int argc, char** argv) {
    std::vector<int> v = create_random_array<int>(100 * MB, 0, 100 * MB);
    std::unordered_set<int> my_set(v.begin(), v.end());
    int rand_num = v[0];

    {
        measure_time m("Serial version");
        int res = 0;
        for (int i = 0; i < v.size(); i++) {
            res += v[i] / rand_num;
        }
        std::cout << "Result = " << res << std::endl;
    }

#pragma omp parallel
    {
#pragma omp single
        std::cout << "Number of threads " << omp_get_num_threads() << std::endl;
    }

    {
        measure_time m("Parallel addition");

        int res = 0;
#pragma omp parallel for shared(v, rand_num) reduction(+ : res) default(none)
        for (int i = 0; i < v.size(); i++) {
            res += v[i] / rand_num;
        }
        std::cout << "Result = " << res << std::endl;
    }

    {
        measure_time m("Serial hashmap lookup");
        int res = 0;

        for (int i = 0; i < v.size(); i++) {
            res += my_set.find(v[i]) != my_set.end();
        }
        std::cout << "Result = " << res << std::endl;
    }

    {
        measure_time m("Parallel hashmap lookup");
        int res = 0;

#pragma omp parallel for shared(v, rand_num, my_set) reduction(+:res) default(none)
        for (int i = 0; i < v.size(); i++) {
            res += my_set.find(v[i]) != my_set.end();
        }
        std::cout << "Result = " << res << std::endl;
    }

    return 0;
}