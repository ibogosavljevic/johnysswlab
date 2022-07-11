#include <vector>
#include <string>
#include <iostream>
#include "likwid.h"

template <int stride = 1>
double test_read(double* in, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i+=stride) {
        sum += in[i];
    }

    return sum;
}

template <int stride = 1>
void test_write(double* in, int n, double v) {
    for (int i = 0; i < n; i+=stride) {
        in[i] = v;
    }
}

static void escape(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}

static void clobber() {
    asm volatile("" : : : "memory");
}

std::string get_test_suffix(int n, int repeat_count) {
    return "_" + std::to_string(n) + "_" + std::to_string(repeat_count);
}


int main(int argc, char** argv) {
    int start_size = 8 * 1024;
    int end_size = 256 * 1024 * 1024;

    LIKWID_MARKER_INIT;

    for (int n = start_size; n <= end_size; n *= 2) {
        std::vector<double> test_vec(n, static_cast<double>(n));
        int repeat_count = (end_size / n) * 4;
        std::string read_test_name = "READ" + get_test_suffix(n, repeat_count);
        std::string write_test_name = "WRITE" + get_test_suffix(n, repeat_count);
        std::string read2_test_name = "READ_STRIDE2" + get_test_suffix(n, repeat_count);
        std::string write2_test_name = "WRITE_STRIDE2" + get_test_suffix(n, repeat_count);

        double sum = 0.0;
        LIKWID_MARKER_START(read_test_name.c_str());
        for (int i = 0; i < repeat_count; i++) {
            sum += test_read<1>(&test_vec[0], n);
            clobber();
        }
        LIKWID_MARKER_STOP(read_test_name.c_str());

        std::cout << "sum = " << sum << std::endl;

        LIKWID_MARKER_START(write_test_name.c_str());
        for (int i = 0; i < repeat_count; i++) {
            test_write<1>(&test_vec[0], n, static_cast<double>(n + 1));
            clobber();
        }
        LIKWID_MARKER_STOP(write_test_name.c_str());

        sum = 0.0;
        LIKWID_MARKER_START(read2_test_name.c_str());
        for (int i = 0; i < repeat_count; i++) {
            sum += test_read<2>(&test_vec[0], n);
            clobber();
        }
        LIKWID_MARKER_STOP(read2_test_name.c_str());

        std::cout << "sum = " << sum << std::endl;

        LIKWID_MARKER_START(write2_test_name.c_str());
        for (int i = 0; i < repeat_count; i++) {
            test_write<2>(&test_vec[0], n, static_cast<double>(n + 1));
            clobber();
        }
        LIKWID_MARKER_STOP(write2_test_name.c_str());


    }

    LIKWID_MARKER_CLOSE;
}