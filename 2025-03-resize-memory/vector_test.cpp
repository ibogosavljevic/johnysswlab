#include <vector>
#include <iostream>
#include <random>
#include "likwid.h"
#include "vector.h"

std::vector<float> generate_random_vector(std::size_t size) {
    std::vector<float> vec;
    vec.reserve(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    for (std::size_t i = 0; i < size; ++i) {
        vec.push_back(dis(gen));
    }
    return vec;
}

template<typename Vec>
void run_test(Vec& v, const std::vector<float> & input, const char * name) {
    LIKWID_MARKER_START(name);
    for (size_t i = 0; i < input.size(); i++) {
        v.push_back(input[i]);
    }
    LIKWID_MARKER_STOP(name);
}

template<typename Vec0, typename Vec1>
void compare_vectors(const Vec0& v0, const Vec1& v1) {
    if (v0.size() != v1.size()) {
        std::cout << "Size not same\n";
        return;
    }

    for (size_t i = 0; i < v0.size(); i++) {
        if (v0[i] != v1[i]) {
            std::cout << "Difference at " << i << "\n";
            return;
        }
    }

    std::cout << "Same\n";
}

int main() {
    std::vector<float> std_vector, input_vector = generate_random_vector(256*1024*1024);
    jsl::vector<float, jsl::simple_allocator> jsl_vector;
    jsl::vector<float, jsl::resize_allocator> jsl_resize_vector;
#ifdef JSL_USE_JEMALLOC
    jsl::vector<float, jsl::resize_allocator_jemalloc> jsl_resize_jemalloc_vector;
#endif
#ifdef JSL_USE_POSIX
    jsl::vector<float, jsl::resize_allocator_posix> jsl_resize_posix_vector;
#endif
    LIKWID_MARKER_INIT;

    run_test(std_vector, input_vector, "std_vector");
    run_test(jsl_vector, input_vector, "jsl_vector");
    run_test(jsl_resize_vector, input_vector, "jsl_resize_vector");
#ifdef JSL_USE_JEMALLOC
    run_test(jsl_resize_jemalloc_vector, input_vector, "jsl_resize_jemalloc_vector");
#endif
#ifdef JSL_USE_POSIX
    run_test(jsl_resize_posix_vector, input_vector, "jsl_resize_posix_vector");
#endif

    compare_vectors(std_vector, input_vector);
    compare_vectors(jsl_vector, input_vector);
    compare_vectors(jsl_resize_vector, input_vector);
#ifdef JSL_USE_JEMALLOC
    compare_vectors(jsl_resize_jemalloc_vector, input_vector);
#endif
#ifdef JSL_USE_POSIX
    compare_vectors(jsl_resize_posix_vector, input_vector);
#endif


    LIKWID_MARKER_CLOSE;
}