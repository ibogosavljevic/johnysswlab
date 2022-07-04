#include "likwid.h"
#include <string>
#include <cstdlib>

template <typename T>
void rotate_matrix_write(T* out, T* in, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            out[i * n + j] = in[j * n + i];
        }
    }
}

template <typename T>
void rotate_matrix_read(T* out, T* in, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            out[j * n + i] = in[i * n + j];
        }
    }
}

template <typename T>
void fill_matrix(T* out, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            out[j * n + i] = i * n + j;
        }
    }
}

int MATRIX_SIZE = 1024 + 128;

int main(int argc, char** argv) {
    LIKWID_MARKER_INIT;

    double* in1 = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double* in2 = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));

    LIKWID_MARKER_CLOSE;
}