#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>
#include "likwid.h"

static void escape(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}

static void clobber() {
    asm volatile("" : : : "memory");
}


template<typename T, int p>
T pow_of_2(T a) {
    if constexpr (p == 1) {
        return a;
    } else {
        T t = pow_of_2<T, p - 1>(a);
        return t * t;
    }
}

template<typename T, int p>
void pow_of_2_array(T* in, T* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = pow_of_2<T, p>(in[i]);
    }
}

std::string get_test_name(int n, int repeat_count, int p) {
    return "TEST_" + std::to_string(n) + "_" + std::to_string(p) + "_" + std::to_string(repeat_count);
}

std::vector<double> initialize_test_array(int n) {
    std::vector<double> result(n, 1.0);

    for (int i = 0; i < n; i++) {
        double diff = 0.5 / i;
        if ((rand() % 2) == 0) {
            diff = -diff;
        }
        result[i] += diff;
    }

    return result;
}

template<int p>
void run_test(int n, int repeat_count, double* in, double* out) {
    std::string name = get_test_name(n, repeat_count, p);
    LIKWID_MARKER_START(name.c_str());
    for (int i = 0; i < repeat_count; ++i) {
        pow_of_2_array<double, p>(in, out, n);
        clobber();
    }
    LIKWID_MARKER_STOP(name.c_str());
}

int main(int argc, char** argv) {
    LIKWID_MARKER_INIT;

    int start_n = 4*1024;
    int end_n = 256*1024*1024;

    for (int n = start_n; n <= end_n; n *= 2) {
        int repeat_count = (end_n / n) * 4;

        std::cout << "Running test for array size " << n << std::endl;

        std::vector<double> in = initialize_test_array(n);
        std::vector<double> out(n);

        run_test<1>(n, repeat_count, &in[0], &out[0]);
        run_test<4>(n, repeat_count, &in[0], &out[0]);
        run_test<7>(n, repeat_count, &in[0], &out[0]);
        run_test<10>(n, repeat_count, &in[0], &out[0]);
        run_test<13>(n, repeat_count, &in[0], &out[0]);
        run_test<16>(n, repeat_count, &in[0], &out[0]);
        run_test<19>(n, repeat_count, &in[0], &out[0]);
        run_test<22>(n, repeat_count, &in[0], &out[0]);
        run_test<25>(n, repeat_count, &in[0], &out[0]);
        run_test<28>(n, repeat_count, &in[0], &out[0]);
    }

    LIKWID_MARKER_CLOSE;
    return 0;
}
