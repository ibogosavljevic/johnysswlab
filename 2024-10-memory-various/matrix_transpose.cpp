#include "../common/argparse.h"
#include "likwid.h"

void matrix_transpose_tiled_in(float* out, const float* in, size_t n) {
    const size_t TILE_SIZE = 16;
    const size_t ii_end = n / TILE_SIZE * TILE_SIZE;
    const size_t jj_end = n / TILE_SIZE * TILE_SIZE;
    
    for (size_t ii = 0; ii < ii_end; ii+=TILE_SIZE) {
        for (size_t jj = 0; jj < jj_end; jj+=TILE_SIZE) {
            for (size_t i = 0; i < TILE_SIZE; i++) {
                for (size_t j = 0; j < TILE_SIZE; j++) {
                    out[(jj + j)* n + ii + i] = in[(ii + i) * n + jj + j];
                }
            }
        }

        // Drain loop for the last few columns in the matrix
        for(size_t i = ii; i < ii + TILE_SIZE; i++) {
            for (size_t j = jj_end; j < n; j++) {
                out[j*n + i] = in[i * n + j];
            }
        }
    }

    // Drain loop for the last few rows in the matrix
    for (size_t i = ii_end; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            out[j*n + i] = in[i * n + j];
        }
    }
}


void matrix_transpose_tiled_out(float* out, const float* in, size_t n) {
    const size_t TILE_SIZE = 16;
    const size_t ii_end = n / TILE_SIZE * TILE_SIZE;
    const size_t jj_end = n / TILE_SIZE * TILE_SIZE;
    
    for (size_t ii = 0; ii < ii_end; ii+=TILE_SIZE) {
        for (size_t jj = 0; jj < jj_end; jj+=TILE_SIZE) {
            for (size_t i = 0; i < TILE_SIZE; i++) {
                for (size_t j = 0; j < TILE_SIZE; j++) {
                    out[(ii + i)* n + jj + j] = in[(jj + j) * n + ii + i];
                }
            }
        }

        // Drain loop for the last few columns in the matrix
        for(size_t i = ii; i < ii + TILE_SIZE; i++) {
            for (size_t j = jj_end; j < n; j++) {
                out[i*n + j] = in[j * n + i];
            }
        }
    }

    // Drain loop for the last few rows in the matrix
    for (size_t i = ii_end; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            out[i*n + j] = in[j * n + i];
        }
    }
}

void matrix_transpose_out_cstyle_tiled(float** out, float** in, size_t n) {
    const size_t TILE_SIZE = 16;
    const size_t ii_end = n / TILE_SIZE * TILE_SIZE;
    const size_t jj_end = n / TILE_SIZE * TILE_SIZE;
    
    for (size_t ii = 0; ii < ii_end; ii+=TILE_SIZE) {
        for (size_t jj = 0; jj < jj_end; jj+=TILE_SIZE) {
            for (size_t i = 0; i < TILE_SIZE; i++) {
                for (size_t j = 0; j < TILE_SIZE; j++) {
                    out[ii + i][jj + j] = in[jj + j][ii + i];
                }
            }
        }

        // Drain loop for the last few columns in the matrix
        for(size_t i = ii; i < ii + TILE_SIZE; i++) {
            for (size_t j = jj_end; j < n; j++) {
                out[i][j] = in[j][i];
            }
        }
    }

    // Drain loop for the last few rows in the matrix
    for (size_t i = ii_end; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            out[i][j] = in[j][i];
        }
    }
}

template <typename T>
void fill_buffer(T* buff, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        buff[i] = static_cast<T>(rand());
    }
}

static void clobber() {
    asm volatile("" : : : "memory");
}

template <typename T>
void assert_buffers_equal(T const * const buff0, T const * const buff1, size_t const size) {
    for (size_t i = 0; i < size; ++i) {
        if (buff0[i] != buff1[i]) {
            std::cout << "Buffers not equal at position " << i << std::endl;
            return;
        }
    }
    std::cout << "Buffers equal\n";
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

float** allocate_matrix(size_t rows, size_t columns) {
    float** res = (float**) malloc(rows * sizeof(float*));
    float* m = (float*) malloc(rows * columns * sizeof(float));

    for (size_t i = 0; i < rows; i++) {
        res[i] = m + i * columns;
        std::memset(res[i], 0, columns * sizeof(float));
    }

    return res;
}

void free_matrix(float** m) {
    free(m[0]);
    free(m);
}

void assert_matrices_equal(float* in0, float** in1, size_t rows, size_t columns) {
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < columns; j++) {
            if (in0[i * columns + j] != in1[i][j]) {
                std::cout << "Matrices not equal at position [" << i << ", " << j << "]" << std::endl;
                return;
            }
        }
    }
    std::cout << "Matrices same\n";
}

void copy_matrix(float** dest, float* src, size_t rows, size_t columns) {
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < columns; j++) {
            dest[i][j] = src[i * columns + j];
        }
    }
}

using namespace argparse;

int main(int argc, const char* argv[]) {
    ArgumentParser parser("matrix_rotate", "matrix_rotate");

    parser.add_argument("-d", "--dimension", "Matrix dimension", true);

    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return false;
    }
    
    size_t matrix_dim = 1024;

    if (parser.exists("d")) {
        size_t new_matrix_dim = parser.get<size_t>("d");
        if (new_matrix_dim > 0) {
            matrix_dim = new_matrix_dim;
        }
    }

    LIKWID_MARKER_INIT;

    size_t matrix_size = matrix_dim * matrix_dim;
    size_t repeat_count = 1024 * 1024 * 1024 / matrix_size;

    float* in_matrix = (float*) malloc(matrix_size * sizeof(float));
    float** in_matrix_full = allocate_matrix(matrix_dim, matrix_dim);
    float* matrix2 = (float*) malloc(matrix_size * sizeof(float));
    float* matrix3 = (float*) malloc(matrix_size * sizeof(float));
    float** out_matrix_full = allocate_matrix(matrix_dim, matrix_dim);

    fill_buffer<float>(in_matrix, matrix_size);
    copy_matrix(in_matrix_full, in_matrix, matrix_dim, matrix_dim);
    std::memset(matrix2, 0, matrix_size * sizeof(float));
    std::memset(matrix3, 0, matrix_size * sizeof(float));

    std::cout << "Matrix dimension " << matrix_dim << ", repeat count " << repeat_count << std::endl;

    run_test(repeat_count, "tiled_in_linear", [&] ()-> void { matrix_transpose_tiled_in(matrix2, in_matrix, matrix_dim); });
    run_test(repeat_count, "tiled_out_linear", [&] ()-> void { matrix_transpose_tiled_out(matrix3, in_matrix, matrix_dim); });
    run_test(repeat_count, "tiled_full_out_linear", [&] ()-> void { matrix_transpose_out_cstyle_tiled(out_matrix_full, in_matrix_full, matrix_dim); });

    assert_buffers_equal(matrix2, matrix3, matrix_size);
    assert_matrices_equal(matrix2, out_matrix_full, matrix_dim, matrix_dim);

    free(in_matrix);
    free(matrix2);
    free(matrix3);
    free_matrix(in_matrix_full);
    free_matrix(out_matrix_full);

    LIKWID_MARKER_CLOSE;
}

