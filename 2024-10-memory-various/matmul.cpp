#include "../common/argparse.h"
#include <cstring>
#include <likwid.h>

void matrix_mul(double* c, double* a, double* b, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            c[i * n + j] = 0.0;
            for (int k = 0; k < n; ++k) {
                c[i * n + j] += a[i * n + k] + b[k * n + j];
            }
        }
    }
}

void matrix_mul_transposed(double* c, double* a, double* b, int n) {
    double* b_transposed = (double*) malloc(sizeof(double) * n * n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            b_transposed[i*n + j] = b[j*n + i];
        }
    }

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            c[i * n + j] = 0.0;
            for (int k = 0; k < n; ++k) {
                c[i * n + j] += a[i * n + k] + b_transposed[j * n + k];
            }
        }
    }

    free(b_transposed);
}

static void clobber() {
    asm volatile("" : : : "memory");
}


template <typename FUNC>
void run_test(int repeat_count, const std::string& name, FUNC f) {
    LIKWID_MARKER_START(name.c_str());
    for (int i = 0; i < repeat_count; ++i) {
        f();
        clobber();
    }
    
    LIKWID_MARKER_STOP(name.c_str());
}

template <typename T>
void fill_buffer(T* buff, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        buff[i] = static_cast<T>(rand());
    }
}

template <typename T>
bool buffers_same(T* a, T* b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            std::cout << "Buffers different at position " << i << "\n";
            return false;
        }
    }
    std::cout << "Buffers same\n";
    return true;
}

using namespace argparse;

int main(int argc, const char* argv[]) {
    ArgumentParser parser("matmul", "matmul");

    parser.add_argument("-d", "--dimension", "Matrix dimension", true);

    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return 0;
    }
    
    size_t matrix_dim = 1024;

    if (parser.exists("d")) {
        size_t new_matrix_dim = parser.get<size_t>("d");
        if (new_matrix_dim > 0) {
            matrix_dim = new_matrix_dim;
        }
    }

    size_t const matrix_size = matrix_dim * matrix_dim;
    size_t repeat_count = std::max<size_t>(256 * 1024 * 1024 / (matrix_size * matrix_dim), 1);

    std::cout << "Dimension " << matrix_dim << ", repeat count " << repeat_count << "\n";

    double * a = (double*) malloc(matrix_size * sizeof(double));
    double * b = (double*) malloc(matrix_size * sizeof(double));
    double * c0 = (double*) malloc(matrix_size * sizeof(double));
    double * c1 = (double*) malloc(matrix_size * sizeof(double));

    fill_buffer(a, matrix_size);
    fill_buffer(b, matrix_size);
    fill_buffer(c0, matrix_size);
    fill_buffer(c1, matrix_size);

    LIKWID_MARKER_INIT;

    run_test(repeat_count, "regular", [&]() -> void { matrix_mul(c0, a, b, matrix_dim); });
    run_test(repeat_count, "transposed", [&]() -> void { matrix_mul_transposed(c1, a, b, matrix_dim); });

    buffers_same(c0, c1, matrix_size);

    LIKWID_MARKER_CLOSE;
}