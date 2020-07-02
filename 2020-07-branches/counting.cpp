
#include "utils.h"
#include "measure_time.h"
#include "argparse.h"


typedef int count_bigger_than_limit_t(int* array, int n, int limit);


__attribute__ ((optimize(FUNC_OPT)))
int count_bigger_than_limit_regular(int* array, int n, int limit) {
    int limit_cnt = 0;
    for (int i = 0; i < n; i++) {
        if (array[i] > limit) {
            limit_cnt++;
        }
    }

    return limit_cnt;
}

__attribute__ ((optimize(FUNC_OPT)))
int count_bigger_than_limit_branchless(int* array, int n, int limit) {
    int limit_cnt[] = { 0, 0 };
    for (int i = 0; i < n; i++) {
        limit_cnt[array[i] > limit]++;
    }
    return limit_cnt[1];
}

__attribute__ ((optimize(FUNC_OPT)))
int count_bigger_than_limit_arithmetic(int* array, int n, int limit) {
    int limit_cnt = 0;
    for (int i = 0; i < n; i++) {
        limit_cnt += (array[i] > limit);
    }
    return limit_cnt;
}

__attribute__ ((optimize(FUNC_OPT)))
int count_bigger_than_limit_cmove(int* array, int n, int limit) {
    int limit_cnt = 0;
    int new_limit_cnt;
    for (int i = 0; i < n; i++) {
        new_limit_cnt = limit_cnt + 1;
#if defined(__MIPSEL)
        __asm__ (
            "sub $8, %[array_i], %[limit];"
            "sra $8, $8, 31;"
            "movz %[limit_cnt], %[new_limit_cnt], $8;"
                : [limit_cnt] "+&r"(limit_cnt)
            : [array_i] "r"(array[i]), [new_limit_cnt] "r"(new_limit_cnt), [limit] "r"(limit)
            : "cc", "$8"
            );

#elif defined(__x86_64)
        __asm__ (
            "cmp %[array_i], %[limit];"
            "cmovbe %[new_limit_cnt], %[limit_cnt];"
	        : [limit_cnt] "+&r"(limit_cnt)
            : [array_i] "g"(array[i]), [new_limit_cnt] "g"(new_limit_cnt), [limit] "r"(limit)
            : "cc"
	    );
#else
#error Unimplemented cmov
#endif
    }
    return limit_cnt;
}

#define likely(x)       __builtin_expect(!!(x),1)
#define unlikely(x)     __builtin_expect(!!(x),0)

int count_bigger_than_limit_likely(int* array, int n, int limit) {
    int limit_cnt = 0;
    for (int i = 0; i < n; i++) {
        if (likely(array[i] > limit)) {
            limit_cnt++;
        }
    }

    return limit_cnt;
}

int count_bigger_than_limit_unlikely(int* array, int n, int limit) {
    int limit_cnt = 0;
    for (int i = 0; i < n; i++) {
        if (unlikely(array[i] > limit)) {
            limit_cnt++;
        }
    }

    return limit_cnt;
}

__attribute__ ((optimize(FUNC_OPT)))
int count_bigger_than_limit_joint_simple(int* array, int n, int limit) {
    int limit_cnt = 0;
    for (int i = 0; i < n/2; i+=2) {
        if (array[i] > limit && array[i + 1] > limit) {
            limit_cnt++;
        }
    }
    return limit_cnt;
}

__attribute__ ((optimize(FUNC_OPT)))
int count_bigger_than_limit_joint_arithmetic(int* array, int n, int limit) {
    int limit_cnt = 0;
    for (int i = 0; i < n/2; i+=2) {
        if (array[i] > limit & array[i + 1] > limit) {
            limit_cnt++;
        }
    }
    return limit_cnt;
}

using namespace argparse;

int main(int argc, const char** argv) {
    count_bigger_than_limit_t* count_func = nullptr;
    std::string count_func_name;

    int searches = 500;
    int arr_len = 10000000;
    float condition = 0.5;

    std::vector<std::pair<std::string, count_bigger_than_limit_t*>> count_funcs = {
         { "regular", count_bigger_than_limit_regular },
         { "cmov", count_bigger_than_limit_cmove},
         { "arithmetic", count_bigger_than_limit_arithmetic },
         { "branchless", count_bigger_than_limit_branchless },
         { "likely", count_bigger_than_limit_likely },
         { "unlikely", count_bigger_than_limit_unlikely },
         { "joint_simple", count_bigger_than_limit_joint_simple },
         { "joint_arithmetic", count_bigger_than_limit_joint_arithmetic },
    };
    ArgumentParser parser("test123", "123");

    parser.add_argument("-t", "--test", "Name of the test to run", true);
    parser.add_argument("-s", "--searches", "Number of searches to perform", false);
    parser.add_argument("-l", "--length", "Array length", false);
    parser.add_argument("-c", "--condition", "Probability of condition being true (0-1)", false);
    
    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return -1;
    }
    
    if (parser.exists("t")) {
        std::string test_name = parser.get<std::string>("t");
        std::cout << "test name : " << test_name << std::endl;
        for (const auto& param: count_funcs) {
            if (param.first == test_name) {
                count_func_name = std::move(test_name);
                count_func = param.second;
                break;
            }
        }
        if (count_func == nullptr) {
            std::cout << "Unknown value for --test\n";
            return -1;
        }   
    } else {
        std::cout << "Parameter --test needed. Possible values for paramter: " << "\n";
        for (const auto& param: count_funcs) {
            std::cout << "--test " << param.first << "\n";
        }
        return -1;
    }

    if (parser.exists("s")) {
        int searches_cmdl = parser.get<int>("s");
        if (searches_cmdl > 0 && searches_cmdl <= 10000000) {
            searches = searches_cmdl;
        }
    }

    if (parser.exists("l")) {
        int arr_len_cmdl = parser.get<int>("l");
        if (arr_len_cmdl >= 1000 && arr_len_cmdl <= 100000000) {
            arr_len = arr_len_cmdl;
        }
    }

    if (parser.exists("c")) {
        float condition_cmdl = parser.get<float>("c");
        if (condition_cmdl >= 0.0 && condition_cmdl <= 1.0) {
            condition = condition_cmdl;
        }
    }

    int res;
    std::vector<int> arr = create_random_array<int>(arr_len, 0, arr_len);

    std::cout << "Running test: " << count_func_name << ", on array length " << arr_len << 
                 ", num of searches " << searches << std::endl; 
    std::cout << "Conditon probability: " << 100.0 * condition << "%" << std::endl;
    {
        measure_time m("Algorithm");
        for (int i = 0; i < searches; i++) {
            res = count_func(arr.data(), arr_len, arr_len * condition);
        }
    }

    std::cout << (res * 100.0) / arr_len << "% of data fulfills the criterium\n";
/*
    {
        measure_time m("Condition always happens");
        for (int i = 0; i < searches; i++) {
            res = count_func(arr.data(), arr_len, arr_len);
            escape(&res);
        }
    }

    {
        measure_time m("Condition never happens");
        for (int i = 0; i < searches; i++) {
            res = count_func(arr.data(), arr_len, arr_len);
            escape(&res);
        }
    }
*/

    return 0;
}
