#include "likwid.h"
#include <string>
#include <cstdlib>
#include <iostream>

template <typename T>
void rotate_matrix_write(T* out, T* in, int n, int unused_count) {
    int r = n + unused_count;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            out[i * r + j] = in[j * r + i];
        }
    }
}

template <typename T>
void rotate_matrix_read(T* out, T* in, int n, int unused_count) {
    int r = n + unused_count;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            out[j * r + i] = in[i * r + j];
        }
    }
}

template <typename T>
void rotate_matrix_write_nontemporal(T* out, T* in, int n, int unused_count) {
    int r = n + unused_count;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            __builtin_nontemporal_store(in[j * r + i], out + i * r + j);
        }
    }
}

template <typename T>
void fill_matrix(T* out, int n, int unused_count) {
    int r = n + unused_count;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            out[i * r + j] = i * n + j;
        }
    }
}

template <typename T>
T* allocate_matrix(int n, int unused_count) {
    return (T*) malloc(n * (n + unused_count) * sizeof(T));
}

template <typename T>
void free_matrix(T* t) {
    free(t);
}

static constexpr int UNUSED_COUNT = 12;

std::string get_test_suffix(int n, int repeat_count) {
    return "_" + std::to_string(n) + "_" + std::to_string(repeat_count);
}

static void escape(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}

int main(int argc, char** argv) {
    LIKWID_MARKER_INIT;

    int start_n = 32;
    int end_n = 8 * 1024;
    
    for (int n = start_n; n <= end_n; n *= 2) {
        std::cout << "N = " << n << std::endl;
        int repeat_count = (end_n / n) * 16;
        
        int* in_matrix = allocate_matrix<int>(n, UNUSED_COUNT);
        int* out_matrix1 = allocate_matrix<int>(n, UNUSED_COUNT);
        int* out_matrix2 = allocate_matrix<int>(n, UNUSED_COUNT);
        int* out_matrix3 = allocate_matrix<int>(n, UNUSED_COUNT);

        std::string read_name = "READ" + get_test_suffix(n, repeat_count);
        std::string write_name = "WRITE" + get_test_suffix(n, repeat_count);
        std::string write_nontemporal_name = "WRITE_NONTEMPORAL" + get_test_suffix(n, repeat_count);
        fill_matrix<int>(in_matrix, n, UNUSED_COUNT);

        LIKWID_MARKER_START(read_name.c_str());
        for (int i = 0; i < repeat_count; i++) {
            rotate_matrix_read(out_matrix1, in_matrix, n, UNUSED_COUNT);
            escape(out_matrix1);
        }
        LIKWID_MARKER_STOP(read_name.c_str());

        LIKWID_MARKER_START(write_name.c_str());
        for (int i = 0; i < repeat_count; i++) {
            rotate_matrix_write(out_matrix2, in_matrix, n, UNUSED_COUNT);
            escape(out_matrix2);
        }
        LIKWID_MARKER_STOP(write_name.c_str());

        LIKWID_MARKER_START(write_nontemporal_name.c_str());
        for (int i = 0; i < repeat_count; i++) {
            rotate_matrix_write_nontemporal(out_matrix3, in_matrix, n, UNUSED_COUNT);
            escape(out_matrix3);
        }
        LIKWID_MARKER_STOP(write_nontemporal_name.c_str());

        free_matrix(in_matrix);
        free_matrix(out_matrix1);
        free_matrix(out_matrix2);
        free_matrix(out_matrix3);
    }


    LIKWID_MARKER_CLOSE;
}