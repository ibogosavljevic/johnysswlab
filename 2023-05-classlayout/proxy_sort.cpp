#include <cstddef>
#include <algorithm>
#include <cassert>

#include "likwid.h"

#include "complex.h"
#include "complex_soa.h"

template<typename Complex>
void proxy_sort(size_t size) {
    std::vector<Complex> vec_original1 = generate_random<Complex>(size);
    std::vector<Complex> vec_original2 = vec_original1;

    std::string suffix = std::to_string(size) + "_" + std::to_string(sizeof(Complex));
    std::string simple_name = "simple_sort_" + suffix;
    std::string proxy_name = "proxy_sort_" + suffix;

    struct proxy {
        complex_simple n;
        int pos;

        bool operator<(const proxy& other) {
            return n < other.n;
        }
    };

    LIKWID_MARKER_START(proxy_name.c_str());

    std::vector<Complex> vec_copy = vec_original1;
    std::vector<proxy> vec_small(size);

    for (int i = 0; i < size; ++i) {
        vec_small[i].n.re = vec_copy[i].re;
        vec_small[i].n.im = vec_copy[i].im;
        vec_small[i].pos = i;
    }

    std::sort(vec_small.begin(), vec_small.end());

    for (int i = 0; i < size; ++i) {
        vec_original1[i] = vec_copy[vec_small[i].pos];
    }

    LIKWID_MARKER_STOP(proxy_name.c_str());

    LIKWID_MARKER_START(simple_name.c_str());
    std::sort(vec_original2.begin(), vec_original2.end());
    LIKWID_MARKER_STOP(simple_name.c_str());

    assert(vec_original1 == vec_original2);

}

int main() {
    LIKWID_MARKER_INIT;

    std::vector<size_t> sizes = {
        2500, 5000, 10000,
        25000, 50000, 100000,
        250000, 500000, 1000000,
        2500000, 5000000, 10000000,
    };

    for (size_t i = 0; i < sizes.size(); i++) {
        size_t size = sizes[i];
        std::cout << "Running for size " << size << std::endl;
        proxy_sort<complex_t<0, 30>>(size);
        proxy_sort<complex_t<0, 34>>(size);
        proxy_sort<complex_t<0, 38>>(size);
        proxy_sort<complex_t<0, 42>>(size);
        proxy_sort<complex_t<0, 48>>(size);
        proxy_sort<complex_t<0, 52>>(size);
    }

    LIKWID_MARKER_CLOSE;
}