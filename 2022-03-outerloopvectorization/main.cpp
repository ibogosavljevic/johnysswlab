#include <cstdlib>
#include <algorithm>
#include <iostream>
#include "likwid.h"
#include <immintrin.h>

// Must be divisible by eight
#define N 2000
#define REPEAT_COUNT 100

static void clobber() {
    asm volatile("" : : : "memory");
}

void print_matrix(float A[N][N]) {
    for (int i = 0; i < N; i++) {
        std::cout << "";
        for (int j = 0; j < N; j++) {
            std::cout << A[i][j] << "|";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void fill_matrix(float A[N][N]) {
    float MAX = (float) RAND_MAX;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = rand() / MAX;
        }
    }
}


void compare_matrices(float m1[N][N], float m2[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (m1[i][j] != m2[i][j]) {
                std::cout << "Not same\n";

                print_matrix(m1);
                print_matrix(m2);
                return;
            }
        }
    }
    std::cout << "Same\n";
}

float res[N][N];

float find_shortest_path_scalar(float A[N][N]) {

    res[0][0] = A[0][0];
    
    for (int i = 1; i < N; i++) {
        res[0][i] = res[0][i - 1] + A[0][i];
        res[i][0] = res[i - 1][0] + A[i][0];
    }

    for (int i = 1; i < N; i++) {
        for (int j = 1; j < N; j++) {
            float min = std::min(res[i - 1][j], res[i][j - 1]);

            res[i][j] = min + A[i][j];
        }
    }

    return res[N-1][N-1];
}


float find_shortest_path_intrinsics(float A[N][N]) {
    res[0][0] = A[0][0];
    
    for (int i = 1; i < N; i++) {
        res[0][i] = res[0][i - 1] + A[0][i];
        res[i][0] = res[i - 1][0] + A[i][0];
    }

    int end = ((N - 1) / 8) * 8;

    __m256i indexes = _mm256_set_epi32(7*N, 6*N + 1, 5*N + 2, 4*N + 3, 3*N + 4, 2*N + 5, N + 6, 7);

    for (int ii = 1; ii < end; ii += 8) {
        for (int i = 0; i < 8; i++) {
            for (int j = 1; j < (9 - i); j++) {
                int new_i = i + ii;
                float min = std::min(res[new_i - 1][j], res[new_i][j - 1]);

                res[new_i][j] = min + A[new_i][j];
            }
        }

        // Vectorized part
        float* left_ptr = res[ii];
        float* top_ptr = res[ii - 1] + 1;
        float* res_ptr = res[ii] + 1;
        float* a_ptr = A[ii] + 1;
        float res_tmp[8];
        
        __m256 left_val = _mm256_i32gather_ps(left_ptr, indexes, 4);

        for (int j = 1; j < (N - 7); j++) {
            __m256 top_val = _mm256_i32gather_ps(top_ptr, indexes, 4);
            __m256 min_val = _mm256_min_ps(left_val, top_val);
            __m256 a_val = _mm256_i32gather_ps(a_ptr, indexes, 4);
            __m256 res_val = _mm256_add_ps(min_val, a_val);
            _mm256_storeu_ps(res_tmp, res_val);

            for (int k = 0; k < 8; k++) {
                res[ii + k][j + 7 - k] = res_tmp[k];
            }
            
            left_val = res_val;

            left_ptr++;
            top_ptr++;
            res_ptr++;
            a_ptr++;
        }

        for (int i = 0; i < 8; i++) {
            for (int j = (N - i); j < N; j++) {
                int new_i = i + ii;
                float min = std::min(res[new_i - 1][j], res[new_i][j - 1]);

                res[new_i][j] = min + A[new_i][j];

            }
        }

    }

    // Drain loop
    for (int i = end; i < N; i++) {
        for (int j = 1; j < N; j++) {
            float min = std::min(res[i - 1][j], res[i][j - 1]);

            res[i][j] = min + A[i][j];
        }
    }

    return res[N-1][N-1];
}



void left_dependency_scalar(float in[N][N], float out[N][N]) {
    for (int i = 0; i < N; i++) {
        out[i][0] = in[i][0];
        for (int j = 1; j < N; j++) {
            out[i][j] = out[i][j - 1] + in[i][j];
        }
    }
}

#pragma omp declare simd uniform(in, out) linear(i) notinbranch
void outer_loop(float in[N][N], float out[N][N], int i) {
    out[i][0] = in[i][0];
    for (int j = 1; j < N; j++) {
        out[i][j] = out[i][j - 1] + in[i][j];
    }
}


void left_dependency_outer_simd_vector(float in[N][N], float out[N][N]) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        outer_loop(in, out, i);
    }
}

void left_dependency_intrinsics(float in[N][N], float out[N][N]) {
    for (int i = 0; i < N; i+=8) {
        out[i][0] = in[i][0];
        out[i + 1][0] = in[i + 1][0];
        out[i + 2][0] = in[i + 2][0];
        out[i + 3][0] = in[i + 3][0];
        out[i + 4][0] = in[i + 4][0];
        out[i + 5][0] = in[i + 5][0];
        out[i + 6][0] = in[i + 6][0];
        out[i + 7][0] = in[i + 7][0];

        float* in_ptr = in[i] + 1;
        float* out_ptr = out[i] + 1;
        float* out_prev_ptr = out[i];

        __m256i indexes = _mm256_set_epi32(7*N, 6*N, 5*N, 4*N, 3*N, 2*N, N, 0);

        float out_tmp[8];

        __m256 out_prev_val = _mm256_i32gather_ps(out_prev_ptr, indexes, 4);
        for (int j = 1; j < N; j++) {
            __m256 in_val = _mm256_i32gather_ps(in_ptr, indexes, 4);
            __m256 out_val = _mm256_add_ps(in_val, out_prev_val);
            _mm256_storeu_ps(out_tmp, out_val);
            out_prev_val = out_val;

            for (int i = 0; i < 8; i++) {
                out_ptr[i * N] = out_tmp[i];
            }

            out_ptr++;
            out_prev_ptr++;
            in_ptr++;
        }
    }
}


float A[N][N];

float out1[N][N];
float out2[N][N];

int main(int argc, char** argv) {
    LIKWID_MARKER_INIT;

    fill_matrix(A);

    float shortest1;
    float shortest2;

    LIKWID_MARKER_START("LeftTopDependencyScalar");
    for (int i = 0; i < REPEAT_COUNT; i++) {
        shortest1 = find_shortest_path_scalar(A);
        clobber();
    }
    LIKWID_MARKER_STOP("LeftTopDependencyScalar");

    LIKWID_MARKER_START("LeftTopDependencyVector");
    for (int i = 0; i < REPEAT_COUNT; i++) {
        shortest2 = find_shortest_path_intrinsics(A);
        clobber();
    }
    LIKWID_MARKER_STOP("LeftTopDependencyVector");

    if (shortest1 != shortest2) {
        std::cout << "Not same " << shortest1 << ", " << shortest2 << "\n";
    } else {
        std::cout << "Same\n";
    }

    LIKWID_MARKER_START("LeftDependencyScalar");
    for (int i = 0; i < REPEAT_COUNT; i++) {
        left_dependency_scalar(A, out1);
        clobber();
    }
    LIKWID_MARKER_STOP("LeftDependencyScalar");

    LIKWID_MARKER_START("LeftDependencyVector");
    for (int i = 0; i < REPEAT_COUNT; i++) {
        left_dependency_intrinsics(A, out2);
        clobber();
    }
    LIKWID_MARKER_STOP("LeftDependencyVector");

    compare_matrices(out1, out2);

    LIKWID_MARKER_CLOSE;
}
