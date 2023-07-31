#include <atomic>
#include <mutex>
#include <thread>
#include <iostream>
#include "utils.h"
#include "likwid.h"

static constexpr int MB = 1024 * 1024;

struct accum {
    int val;
    int padding[31];
};

int main(int argc, char* argv[]) {
    std::vector<int> v = create_random_array<int>(100 * MB, 0, 100 * MB);
    int num_threads = 4;  // std::thread::hardware_concurrency();
    int count_per_threads = v.size() / num_threads;
    int rand_num = v[0] == 0 ? 1 : v[0];

    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;

    {
        std::vector<std::thread> threads;
        threads.reserve(num_threads);
        volatile accum result_arr[64];

        auto f = [&](int thread_id, int start, int stop) {
            LIKWID_MARKER_START("NO_FALSE_SHARING");
            result_arr[thread_id].val = 0;
            for (int i = start; i < stop; i++) {
                result_arr[thread_id].val += v[i] / rand_num;
            }
            LIKWID_MARKER_STOP("NO_FALSE_SHARING");
        };

        for (int i = 0; i < num_threads; i++) {
            if (i != num_threads - 1) {
                threads.emplace_back(f, i, i * count_per_threads,
                                     (i + 1) * count_per_threads);
            } else {
                threads.emplace_back(f, i, i * count_per_threads, v.size());
            }
        }

        int res = 0;
        for (int i = 0; i < num_threads; i++) {
            threads[i].join();
            res += result_arr[i].val;
        }
        std::cout << "Result = " << res << std::endl;
    }

    {
        std::vector<std::thread> threads;
        threads.reserve(num_threads);
        volatile int result_arr[64];

        auto f = [&](int thread_id, int start, int stop) {
            LIKWID_MARKER_START("FALSE_SHARING");
            result_arr[thread_id] = 0;
            for (int i = start; i < stop; i++) {
                result_arr[thread_id] += v[i] / rand_num;
            }
            LIKWID_MARKER_STOP("FALSE_SHARING");
        };

        for (int i = 0; i < num_threads; i++) {
            if (i != num_threads - 1) {
                threads.emplace_back(f, i, i * count_per_threads,
                                     (i + 1) * count_per_threads);
            } else {
                threads.emplace_back(f, i, i * count_per_threads, v.size());
            }
        }

        int res = 0;
        for (int i = 0; i < num_threads; i++) {
            threads[i].join();
            res += result_arr[i];
        }
        std::cout << "Result = " << res << std::endl;
    }

    LIKWID_MARKER_CLOSE;

    return 0;
}