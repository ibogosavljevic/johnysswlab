#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "likwid.h"

__attribute__((noinline))
void matrix_mul1(double* c, double* a, double* b, int n) {
    LIKWID_MARKER_START("matmul1");
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            c[i * n + j] = 0.0;
            for (int k = 0; k < n; ++k) {
                c[i * n + j] += a[i * n + k] + b[k * n + j];
            }
        }
    }
    LIKWID_MARKER_STOP("matmul1");
}

__attribute__((noinline))
void matrix_mul2(double* c, double* a, double* b, int n) {
    LIKWID_MARKER_START("matmul2");
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            c[i * n + j] = 0.0;
            for (int k = 0; k < n; ++k) {
                c[i * n + j] += a[i * n + k] + b[k * n + j];
            }
        }
    }
    LIKWID_MARKER_STOP("matmul2");

}

__attribute__((noinline))
void matrix_mul3(double* c, double* a, double* b, int n) {
    LIKWID_MARKER_START("matmul3");
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            c[i * n + j] = 0.0;
            for (int k = 0; k < n; ++k) {
                c[i * n + j] += a[i * n + k] + b[k * n + j];
            }
        }
    }
    LIKWID_MARKER_STOP("matmul3");
}

static void clobber() {
    asm volatile("" : : : "memory");
}

void fill_matrix(double* m, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            m[i * n + j] = 1.0 / (i + j + 1);
        }
    }
}

double hash(double* m, int n) {
    double res = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            res += m[i * n + j];
        }
    }
    return res;
}


void* malloc_aligned(size_t s) {
    void* res = 0;
    posix_memalign(&res, 128, s);
    return res;
}

void matrix_test() {
    clock_t start, end;
    int sizes[] = { 512-16, 512, 512+16 };
    int max_size = sizes[2];
    double cpu_time_used;

    double* c = malloc_aligned(max_size * max_size * sizeof(double));
    double* a = malloc_aligned(max_size * max_size * sizeof(double));
    double* b = malloc_aligned(max_size * max_size * sizeof(double));

    fill_matrix(a, max_size);
    fill_matrix(b, max_size);
    fill_matrix(c, max_size);

    // Warm-up run
    //matrix_mul1(c, a, b, sizes[2]);
    //clobber();
    
    start = clock();
    matrix_mul1(c, a, b, sizes[0]);
    clobber();
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Matrix size %d, runtime %f, res = %f\n", sizes[0],cpu_time_used, hash(c, sizes[0]));

    start = clock();
    matrix_mul2(c, a, b, sizes[1]);
    clobber();
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Matrix size %d, runtime %f, res = %f\n", sizes[1],cpu_time_used, hash(c, sizes[1]));

    start = clock();
    matrix_mul3(c, a, b, sizes[2]);
    clobber();
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Matrix size %d, runtime %f, res = %f\n", sizes[2],cpu_time_used, hash(c, sizes[2]));

    free(a);
    free(b);
    free(c);
}


int binary_search1(int *array, int number_of_elements, int key)
{
    int low = 0, high = number_of_elements - 1, mid;
    while (low <= high)
    {
        mid = (low + high) / 2;

        if (array[mid] < key)
            low = mid + 1;
        else if (array[mid] > key)
            high = mid - 1;
        else
            return mid;
    }
    return -1;
}

int binary_search2(int *array, int number_of_elements, int key)
{
    int low = 0, high = number_of_elements - 1, mid;
    while (low <= high)
    {
        mid = (low + high) / 2;

        if (array[mid] < key)
            low = mid + 1;
        else if (array[mid] > key)
            high = mid - 1;
        else
            return mid;
    }
    return -1;
}

int binary_search3(int *array, int number_of_elements, int key)
{
    int low = 0, high = number_of_elements - 1, mid;
    while (low <= high)
    {
        mid = (low + high) / 2;

        if (array[mid] < key)
            low = mid + 1;
        else if (array[mid] > key)
            high = mid - 1;
        else
            return mid;
    }
    return -1;
}

void generate_sorted_array(int * arr, int n) {
    int current = 0;
    for (int i = 0; i < n; i++) {
        arr[i] = current;
        current += 1 + (rand() % 2);
    }
}

void generate_lookup_array(int* arr, int n) {
    for (int i = 0; i < n; ++i) {
        arr[i] = i;
    }

    for (int i = 0; i < n; ++i) {
        int random_index = rand() % n;
        int tmp = arr[i];
        arr[i] = arr[random_index];
        arr[random_index] = tmp;
    }
}

#define MB16 (4 * 1024 * 1024)

void binary_search_test() {
    clock_t start, end;
    int sizes[] = { MB16 - 600, MB16, MB16 + 600 };
    int max_size = sizes[2];
    double cpu_time_used;

    int* sorted_arr = malloc_aligned(sizes[2] * sizeof(int));
    generate_sorted_array(sorted_arr, sizes[2]);
    
    int lookup_arr_len = sorted_arr[sizes[2] - 1];
    int* lookup_arr = malloc_aligned(sizeof(int) * lookup_arr_len);
    generate_lookup_array(lookup_arr, lookup_arr_len);

    int cnt = 0;
    start = clock();
    for (int i = 0; i < lookup_arr_len; ++i) {
        cnt += binary_search1(sorted_arr, sizes[0], lookup_arr[i])!= -1;
    }
    clobber();
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Array size %d, runtime %f, res = %d\n", sizes[0], cpu_time_used, cnt);

    cnt = 0;
    start = clock();
    for (int i = 0; i < lookup_arr_len; ++i) {
        cnt += binary_search2(sorted_arr, sizes[1], lookup_arr[i])!= -1;
    }
    clobber();
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Array size %d, runtime %f, res = %d\n", sizes[1], cpu_time_used, cnt);

    cnt = 0;
    start = clock();
    for (int i = 0; i < lookup_arr_len; ++i) {
        cnt += binary_search3(sorted_arr, sizes[2], lookup_arr[i]) != -1;
    }
    clobber();
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Array size %d, runtime %f, res = %d\n", sizes[2], cpu_time_used, cnt);


    free(sorted_arr);
    free(lookup_arr);
}

int main() {
    LIKWID_MARKER_INIT;
    matrix_test();
    binary_search_test();
    LIKWID_MARKER_CLOSE;
}
