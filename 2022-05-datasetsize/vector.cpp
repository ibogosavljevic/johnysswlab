#include <vector>
#include <string>
#include <cmath>

#include <likwid.h>

static void clobber() {
    asm volatile("" : : : "memory");
}

template <typename T>
void add_vectors_vectorized(std::vector<T>& c, const std::vector<T>& a, const std::vector<T>& b) {
    int size = c.size();
    #pragma clang loop vectorize(enable) unroll(disable)
    for (int i = 0; i < size; i++) {
        c[i] = a[i] + b[i];
    }
    clobber();
}

template <typename T>
void add_vectors_notvectorized(std::vector<T>& c, const std::vector<T>& a, const std::vector<T>& b) {
    int size = c.size();
    #pragma clang loop vectorize(disable) unroll(disable)
    for (int i = 0; i < size; i++) {
        T a_i = a[i];
        T b_i = b[i];
        c[i] = a[i] + b[i];
    }
    clobber();
}


template <typename T>
void distance_vectors_vectorized(std::vector<T>& c, const std::vector<T>& a, const std::vector<T>& b) {
    int size = c.size();
    #pragma clang loop vectorize(enable) unroll(disable)
    for (int i = 0; i < size; i++) {
        T a_i = a[i];
        T b_i = b[i];
        c[i] = std::sqrt(a_i * a_i + b_i * b_i);
    }
    clobber();
}

template <typename T>
void distance_vectors_notvectorized(std::vector<T>& c, const std::vector<T>& a, const std::vector<T>& b) {
    int size = c.size();
    #pragma clang loop vectorize(disable) unroll(disable)
    for (int i = 0; i < size; i++) {
        T a_i = a[i];
        T b_i = b[i];
        c[i] = std::sqrt(a_i * a_i + b_i * b_i);
    }
    clobber();
}


static constexpr int size = 128 * 1024 * 1024;

template <typename T>
void run_test(int size) {
    std::vector<T> out1(size, 0.0);
    std::vector<T> out2(size, 0.0);
    std::vector<T> in1(size, 0.1);
    std::vector<T> in2(size, 0.2);

    std::string add_vectorized = std::string("Add_Vectorized_") + typeid(T).name();
    std::string add_not_vectorized = std::string("Add_Not_Vectorized_") + typeid(T).name();
    std::string distance_vectorized = std::string("Distance_Vectorized_") + typeid(T).name();
    std::string distance_not_vectorized = std::string("Distance_Not_Vectorized_") + typeid(T).name();

    LIKWID_MARKER_START(add_vectorized.c_str());
    add_vectors_vectorized(out1, in1, in2);
    LIKWID_MARKER_STOP(add_vectorized.c_str());

    LIKWID_MARKER_START(add_not_vectorized.c_str());
    add_vectors_notvectorized(out2, in1, in2);
    LIKWID_MARKER_STOP(add_not_vectorized.c_str());

    LIKWID_MARKER_START(distance_vectorized.c_str());
    distance_vectors_vectorized(out1, in1, in2);
    LIKWID_MARKER_STOP(distance_vectorized.c_str());

    LIKWID_MARKER_START(distance_not_vectorized.c_str());
    distance_vectors_notvectorized(out2, in1, in2);
    LIKWID_MARKER_STOP(distance_not_vectorized.c_str());
}

int main(int argc, char* argv[]) {
    LIKWID_MARKER_INIT;

    run_test<float>(size);
    run_test<double>(size);

    LIKWID_MARKER_CLOSE;
    return 0;
}