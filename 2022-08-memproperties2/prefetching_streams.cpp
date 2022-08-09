#include <vector>
#include <string>
#include <iostream>
#include "likwid.h"

static void clobber() {
    asm volatile("" : : : "memory");
}

double add(int i, double* vec) {
    return vec[i];
}

template <typename... Args>
double add(int i, double* vec, Args... args) {
    return vec[i] + add(i, args...);
}

template <typename... Args>
double add_vectors(int repeat_count, std::string name, int n, Args... args) {
    double result = 0.0;
    LIKWID_MARKER_START(name.c_str());
    for (int r = 0; r < repeat_count; ++r) {
        #pragma clang loop vectorize(enable)
        for (int i = 0; i < n; i++) {
            result += add(i, args...);
        }
        clobber();
    }
    LIKWID_MARKER_STOP(name.c_str());

    return result;
}


int main(int argc, char** argv) {
    static constexpr int total_size = 128*1024*1024;
    static constexpr int size = 8*1024*1024;
    static constexpr int offset = 8*1024*1024 - 1024;

    static constexpr int repeat_count = 128;

    LIKWID_MARKER_INIT;

    std::vector<double> vec(total_size, 0.1);
    double* v1  = &vec[0];
    double* v2 = &vec[offset];
    double* v3 = &vec[2 * offset];
    double* v4 = &vec[3 * offset];
    double* v5 = &vec[4 * offset];
    double* v6 = &vec[5 * offset];
    double* v7 = &vec[6 * offset];
    double* v8 = &vec[7 * offset];
    double* v9 = &vec[8 * offset];
    double* v10 = &vec[9 * offset];
    double* v11 = &vec[10 * offset];
    double* v12 = &vec[11 * offset];
    double* v13 = &vec[12 * offset];
    double* v14 = &vec[13 * offset];
    double* v15 = &vec[14 * offset];

    double r;

    r = add_vectors(repeat_count, "vec1", size, v1);
    std::cout << r << std::endl;

    r = add_vectors(repeat_count, "vec2", size, v1, v2);
    std::cout << r << std::endl;
    
    r = add_vectors(repeat_count, "vec3", size, v1, v2, v3);
    std::cout << r << std::endl;

    r = add_vectors(repeat_count, "vec4", size, v1, v2, v3, v4);
    std::cout << r << std::endl;

    r = add_vectors(repeat_count, "vec5", size, v1, v2, v3, v4, v5);
    std::cout << r << std::endl;
    
    r = add_vectors(repeat_count, "vec6", size, v1, v2, v3, v4, v5, v6);
    std::cout << r << std::endl;

    r = add_vectors(repeat_count, "vec7", size, v1, v2, v3, v4, v5, v6, v7);
    std::cout << r << std::endl;

    r = add_vectors(repeat_count, "vec8", size, v1, v2, v3, v4, v5, v6, v7, v8);
    std::cout << r << std::endl;

    r = add_vectors(repeat_count, "vec9", size, v1, v2, v3, v4, v5, v6, v7, v8, v9);
    std::cout << r << std::endl;

    r = add_vectors(repeat_count, "vec10", size, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
    std::cout << r << std::endl;

    r = add_vectors(repeat_count, "vec11", size, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11);
    std::cout << r << std::endl;

    r = add_vectors(repeat_count, "vec12", size, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12);
    std::cout << r << std::endl;

    r = add_vectors(repeat_count, "vec13", size, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13);
    std::cout << r << std::endl;

    r = add_vectors(repeat_count, "vec14", size, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14);
    std::cout << r << std::endl;

    r = add_vectors(repeat_count, "vec15", size, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15);
    std::cout << r << std::endl;

    LIKWID_MARKER_CLOSE;
}