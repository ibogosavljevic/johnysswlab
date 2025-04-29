#include <likwid.h>
#include "common.hpp"
#include <cassert>

void stripe_color(int8_t* image, size_t n, int8_t color_odd, int8_t color_even) {
    for (size_t i = 0; i < n; i++) {
        int8_t color = ((i % 2) == 0) ? color_even : color_odd;
        for (size_t j = 0; j < n; j++) {
            image[j * n + i] = color;
        }
    }
}

void stripe_color_parallelized(int8_t* image, size_t n, int8_t color_odd, int8_t color_even) {
    #pragma omp parallel for shared(n, image, color_even, color_odd) default(none)
    for (size_t i = 0; i < n; i++) {
        int8_t color = ((i % 2) == 0) ? color_even : color_odd;
        for (size_t j = 0; j < n; j++) {
            image[j * n + i] = color;
        }
    }
}

void stripe_color_interchanged(int8_t* image, size_t n, int8_t color_odd, int8_t color_even) {
    for (size_t j = 0; j < n; j++) {
        for (size_t i = 0; i < n; i++) {
            int8_t color = ((i % 2) == 0) ? color_even : color_odd;
            image[j * n + i] = color;
        }
    }
}


void stripe_color_interchanged_no_conditions(int8_t* image, size_t n, int8_t color_odd, int8_t color_even) {
    for (size_t j = 0; j < n; j++) {
        for (size_t i = 0; i < n; i+=2) {
            image[j * n + i] = color_even;
            image[j * n + i + 1] = color_odd;
        }
    }
}

void stripe_color_interchanged_no_conditions_parallelized(int8_t* image, size_t n, int8_t color_odd, int8_t color_even) {
    #pragma omp parallel for shared(n, image, color_even, color_odd) default(none)
    for (size_t j = 0; j < n; j++) {
        for (size_t i = 0; i < n; i+=2) {
            image[j * n + i] = color_even;
            image[j * n + i + 1] = color_odd;
        }
    }
}


int main() {
    static constexpr size_t N = 3840;
    static constexpr size_t BUFF_SIZE = N * N;

    LIKWID_MARKER_INIT;

    int8_t* out1 = aligned::allocate_buffer<int8_t>(BUFF_SIZE);
    int8_t* out2 = aligned::allocate_buffer<int8_t>(BUFF_SIZE);
    int8_t* out3 = aligned::allocate_buffer<int8_t>(BUFF_SIZE);
    int8_t* out4 = aligned::allocate_buffer<int8_t>(BUFF_SIZE);
    int8_t* out5 = aligned::allocate_buffer<int8_t>(BUFF_SIZE);


    set_buffer<int8_t>(out1, BUFF_SIZE, 0);
    set_buffer<int8_t>(out2, BUFF_SIZE, 0);
    set_buffer<int8_t>(out3, BUFF_SIZE, 0);
    set_buffer<int8_t>(out4, BUFF_SIZE, 0);
    set_buffer<int8_t>(out5, BUFF_SIZE, 0);

    run_test(100, "stripe_color", [&]() -> void { stripe_color(out1, N, 1, 5); });
    run_test(100, "stripe_color_parallelized", [&]() -> void { stripe_color_parallelized(out2, N, 1, 5); });
    run_test(100, "stripe_color_interchanged", [&]() -> void { stripe_color_interchanged(out3, N, 1, 5); });
    run_test(100, "stripe_color_interchanged_noconditions", [&]() -> void { stripe_color_interchanged_no_conditions(out4, N, 1, 5); });
    run_test(100, "stripe_color_interchanged_noconditions_parallelized", [&]() -> void { stripe_color_interchanged_no_conditions_parallelized(out5, N, 1, 5); });

    equal(out1, out2, N);
    equal(out1, out3, N);
    equal(out1, out4, N);
    equal(out1, out5, N);

    aligned::free_buffer(out1);
    aligned::free_buffer(out2);
    aligned::free_buffer(out3);
    aligned::free_buffer(out4);
    aligned::free_buffer(out5);

    LIKWID_MARKER_CLOSE;
}