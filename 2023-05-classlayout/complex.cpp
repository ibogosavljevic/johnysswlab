
#include <vector>
#include <algorithm>
#include <string>
#include <typeinfo>
#include <iostream>
#include <cassert>
#include "likwid.h"

#include "complex.h"
#include "complex_soa.h"

static constexpr int TOTAL_OPERATIONS = 10000000;


static void escape(void* p) {
  asm volatile ("" : : "g"(p) : "memory");
}

template <typename Complex>
void run_test(const std::string& prefix, size_t size) {
    
    std::vector<Complex> in1 = generate_random<Complex>(size);
    std::vector<Complex> in2 = generate_random<Complex>(size);
    std::vector<Complex> out(size);

    int repeat_count = TOTAL_OPERATIONS / size;
    std::string name = std::to_string(size) + "_" + std::to_string(sizeof(Complex)) + "_" + std::to_string(offsetof(Complex, im));
    std::string name_multiply = prefix + "_multiply_" + name;
    std::string name_sort = prefix + "_sort_" + name;
    std::string name_binary_search_low_bound = prefix + "_binary_search_" + name;

    LIKWID_MARKER_START(name_multiply.c_str());
    for (int i = 0; i < repeat_count; ++i) {
        multiply(out.data(), in1.data(), in2.data(), size);
        escape(out.data());
    }
    LIKWID_MARKER_STOP(name_multiply.c_str());

    LIKWID_MARKER_START(name_sort.c_str());
    std::sort(out.begin(), out.end());
    LIKWID_MARKER_STOP(name_sort.c_str());

    std::vector<Complex> lookup_values = generate_random<Complex>(TOTAL_OPERATIONS);

    size_t sum = 0;
    LIKWID_MARKER_START(name_binary_search_low_bound.c_str());
    for (int i = 0; i < TOTAL_OPERATIONS; ++i) {
        sum += lower_bound(out.data(), size, lookup_values[i]);
    }
    LIKWID_MARKER_STOP(name_binary_search_low_bound.c_str());
    escape(&sum);

}

void run_test_soa(size_t size) {
    complex_soa in1 = generate_random(size);
    complex_soa in2 = generate_random(size);
    complex_soa out(size);

    int repeat_count = TOTAL_OPERATIONS / size;
    std::string name = std::to_string(size) + "_soa";
    std::string name_multiply = "multiply_" + name;
    std::string name_sort = "sort_" + name;
    std::string name_binary_search_low_bound = "binary_search_" + name;

    LIKWID_MARKER_START(name_multiply.c_str());
    for (int i = 0; i < repeat_count; ++i) {
        multiply(out, in1, in2, size);
        escape(out.re.data());
        escape(out.im.data());
    }
    LIKWID_MARKER_STOP(name_multiply.c_str());

    LIKWID_MARKER_START(name_sort.c_str());
    quicksort(out);
    LIKWID_MARKER_STOP(name_sort.c_str());

    complex_soa lookup_values = generate_random(TOTAL_OPERATIONS);

    size_t sum = 0;
    LIKWID_MARKER_START(name_binary_search_low_bound.c_str());
    for (int i = 0; i < TOTAL_OPERATIONS; ++i) {
        sum += lower_bound(out, size, lookup_values[i]);
    }
    LIKWID_MARKER_STOP(name_binary_search_low_bound.c_str());
    escape(&sum);

}

void verify() {
    constexpr int size = 10;
    complex_soa in_soa1(size), in_soa2(size), out_soa(size);
    std::vector<complex_simple> in1(size), in2(size), out(size);

    for (int i = 0; i < size; ++i) {
        in_soa1.re[i] = in1[i].re = i;
        in_soa1.im[i] = in1[i].im = -i;

        in_soa2.re[i] = in2[i].re = 2 * i;
        in_soa2.im[i] = in2[i].im = -i + 2;
    }

    multiply(out_soa, in_soa1, in_soa2, size);
    multiply(out.data(), in1.data(), in2.data(), size);
    assert(out_soa.equal(out));

    quicksort(out_soa);
    std::sort(out.begin(), out.end());
    assert(out_soa.equal(out));

    for (int i = 0; i < size; i++) {
        assert(in_soa1[i] == in1[i]);
        assert(lower_bound(out_soa, size, in_soa1[i]) == lower_bound(out.data(), size, in1[i]));
    }
}

int main() {
    LIKWID_MARKER_INIT;

    verify();

    std::vector<size_t> sizes = {
        10000, 100000, 1000000, 10000000
    };

    for (size_t i = 0; i < sizes.size(); i++) {
        size_t size = sizes[i];
        run_test_soa(size);
        run_test<complex_packed>("size", size);
        run_test<complex_t<0, 0>>("size", size);
        run_test<complex_t<0, 1>>("size", size);
        run_test<complex_t<0, 3>>("size", size);
        run_test<complex_t<0, 5>>("size", size);
        run_test<complex_t<0, 7>>("size", size);
    }

    std::vector<size_t> sizes2 = {
        10000, 100000, 1000000, 10000000
    };

    for (size_t i = 0; i < sizes2.size(); i++) {
        size_t size = sizes2[i];

        run_test<complex_t<0, 12>>("layout", size);
        run_test<complex_t<1, 11>>("layout", size);
        run_test<complex_t<2, 10>>("layout", size);
        run_test<complex_t<3, 9>>("layout", size);
        run_test<complex_t<4, 8>>("layout", size);
        run_test<complex_t<5, 7>>("layout", size);
        run_test<complex_t<6, 6>>("layout", size);

        run_test<complex_t<0, 20>>("layout", size);
        run_test<complex_t<1, 19>>("layout", size);
        run_test<complex_t<2, 18>>("layout", size);
        run_test<complex_t<3, 17>>("layout", size);
        run_test<complex_t<4, 16>>("layout", size);
        run_test<complex_t<5, 15>>("layout", size);
        run_test<complex_t<6, 14>>("layout", size);
    }


    LIKWID_MARKER_CLOSE;
}