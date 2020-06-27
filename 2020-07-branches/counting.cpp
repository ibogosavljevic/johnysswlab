
#include "utils.h"
#include "measure_time.h"
#include "argparse.h"


int count_bigger_than_limit(int* array, int n, int limit) {
    int limit_cnt = 0;
    for (int i = 0; i < n; i++) {
        if (array[i] > limit) {
            limit_cnt++;
        }
    }

    return limit_cnt;
}

int count_bigger_than_limit_branchless(int* array, int n, int limit) {
    int limit_cnt[] = { 0, 0 };
    for (int i = 0; i < n; i++) {
        limit_cnt[array[i] > limit]++;
    }
    return limit_cnt[1];
}


int main(int argc, char** argv) {
    int searches = 500;
    int arr_len = 10000000;
    int res;
    std::vector<int> arr = create_random_array<int>(arr_len, 0, arr_len);

    {
        measure_time m("searches random");
        for (int i = 0; i < searches; i++) {
            res = count_bigger_than_limit(arr.data(), arr_len, arr_len / 2);
            escape(&res);
        }
    }
    std::cout << "RES = " << res;
    {
        measure_time m("searches unifrom");
        for (int i = 0; i < searches; i++) {
            res = count_bigger_than_limit(arr.data(), arr_len, arr_len);
            escape(&res);
        }
    }
    std::cout << "RES = " << res;

    return 0;
}