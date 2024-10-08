#include <atomic>
#include <mutex>
#include <thread>
#include "measure_time.h"
#include "spinlock.h"
#include "utils.h"

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
    std::mutex mutex;

    {
        measure_time m("Serial version");
        volatile int res = 0;
        for (int i = 0; i < v.size(); i++) {
            res += v[i] / rand_num;
        }
        std::cout << "Result = " << res << std::endl;
    }

    {
        measure_time m("Serial version, with mutexes");
        volatile int res = 0;
        for (int i = 0; i < v.size(); i++) {
            mutex.lock();
            res += v[i] / rand_num;
            mutex.unlock();
        }
        std::cout << "Result = " << res << std::endl;
    }

    {
        measure_time m("Serial version, with spinlocks");
        volatile int res = 0;
        spinlock s;
        for (int i = 0; i < v.size(); i++) {
            s.lock();
            res += v[i] / rand_num;
            s.unlock();
        }
        std::cout << "Result = " << res << std::endl;
    }

    {
        measure_time m("Serial version, with atomic variable");
        std::atomic<int> res;
        res = 0;
        for (int i = 0; i < v.size(); i++) {
            res += v[i] / rand_num;
        }
        std::cout << "Result = " << res << std::endl;
    }

    {
        measure_time m("No locks");
        int res = 0;
        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        auto f = [&](int start, int stop) {
            for (int i = start; i < stop; i++) {
                res += v[i] / rand_num;
            }
        };

        for (int i = 0; i < num_threads; i++) {
            if (i != num_threads - 1) {
                threads.emplace_back(f, i * count_per_threads,
                                     (i + 1) * count_per_threads);
            } else {
                threads.emplace_back(f, i * count_per_threads, v.size());
            }
        }

        for (int i = 0; i < num_threads; i++) {
            threads[i].join();
        }
        std::cout << "Result = " << res << std::endl;
    }

    {
        measure_time m("Mutex");
        int res;
        res = 0;
        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        auto f = [&](int start, int stop) {
            for (int i = start; i < stop; i++) {
                int tmp = v[i] / rand_num;
                mutex.lock();
                res += tmp;
                mutex.unlock();
            }
        };

        for (int i = 0; i < num_threads; i++) {
            if (i != num_threads - 1) {
                threads.emplace_back(f, i * count_per_threads,
                                     (i + 1) * count_per_threads);
            } else {
                threads.emplace_back(f, i * count_per_threads, v.size());
            }
        }

        for (int i = 0; i < num_threads; i++) {
            threads[i].join();
        }
        std::cout << "Result = " << res << std::endl;
    }

    {
        measure_time m("Spinlock");
        int res;
        res = 0;
        std::vector<std::thread> threads;
        threads.reserve(num_threads);
        spinlock s;

        auto f = [&](int start, int stop) {
            for (int i = start; i < stop; i++) {
                int tmp = v[i] / rand_num;
                s.lock();
                res += tmp;
                s.unlock();
            }
        };

        for (int i = 0; i < num_threads; i++) {
            if (i != num_threads - 1) {
                threads.emplace_back(f, i * count_per_threads,
                                     (i + 1) * count_per_threads);
            } else {
                threads.emplace_back(f, i * count_per_threads, v.size());
            }
        }

        for (int i = 0; i < num_threads; i++) {
            threads[i].join();
        }
        std::cout << "Result = " << res << std::endl;
    }

    {
        measure_time m("Atomic lock");
        std::atomic<int> res;
        res = 0;
        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        auto f = [&](int start, int stop) {
            for (int i = start; i < stop; i++) {
                res += v[i] / rand_num;
            }
        };

        for (int i = 0; i < num_threads; i++) {
            if (i != num_threads - 1) {
                threads.emplace_back(f, i * count_per_threads,
                                     (i + 1) * count_per_threads);
            } else {
                threads.emplace_back(f, i * count_per_threads, v.size());
            }
        }

        for (int i = 0; i < num_threads; i++) {
            threads[i].join();
        }
        std::cout << "Result = " << res << std::endl;
    }

    {
        measure_time m("Atomic lock, but every 50th time");
        std::atomic<int> res;
        res = 0;
        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        auto f = [&](int start, int stop) {
            int tmp;
            for (int i = start; i < stop; i += 50) {
                tmp = 0;
                for (int j = i; j < std::min(stop, i + 50); j++) {
                    tmp += v[j] / rand_num;
                }
                res += tmp;
            }
        };

        for (int i = 0; i < num_threads; i++) {
            if (i != num_threads - 1) {
                threads.emplace_back(f, i * count_per_threads,
                                     (i + 1) * count_per_threads);
            } else {
                threads.emplace_back(f, i * count_per_threads, v.size());
            }
        }

        for (int i = 0; i < num_threads; i++) {
            threads[i].join();
        }
        std::cout << "Result = " << res << std::endl;
    }

    {
        measure_time m("Reduction on array, no false sharing");
        std::vector<std::thread> threads;
        threads.reserve(num_threads);
        volatile accum result_arr[64];

        auto f = [&](int thread_id, int start, int stop) {
            result_arr[thread_id].val = 0;
            for (int i = start; i < stop; i++) {
                result_arr[thread_id].val += v[i] / rand_num;
            }
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
        measure_time m("Reduction on array, with false sharing");
        std::vector<std::thread> threads;
        threads.reserve(num_threads);
        volatile int result_arr[64];

        auto f = [&](int thread_id, int start, int stop) {
            result_arr[thread_id] = 0;
            for (int i = start; i < stop; i++) {
                result_arr[thread_id] += v[i] / rand_num;
            }
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

    return 0;
}