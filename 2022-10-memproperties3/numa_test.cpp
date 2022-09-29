#include <iostream>
#include <string>
#include "omp.h"
#include "likwid.h"
#include <sys/mman.h>

static void clobber() {
  asm volatile("" : : : "memory");
}

double* malloc_large_double(size_t count) {
    return (double*) mmap(0, count * sizeof(double), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
}

void free_large_double(double* p, size_t count) {
    munmap(p, count * sizeof(double));
}

void initialize_data_single_threaded(double* values, size_t size) {
    for (size_t i = 0; i < size; i++) {
        values[i] = 1.0 / (1.0 + static_cast<double>(i));
    }
}

void copy_data_multithreaded_static(double* dst, double* src, size_t size, int num_cores) {
    #pragma omp parallel for default(none) shared(src, dst, size) schedule(static) num_threads(num_cores)
    for (size_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }
}

void copy_data_multithreaded_random(double* dst, double* src, size_t size, int num_cores) {
    static constexpr int PAGE_SIZE = 4 * 1024 / sizeof(double);
    size_t current_index = 0;

    #pragma omp parallel default(none) shared(src, dst, size, current_index) num_threads(num_cores)
    {
        srand(int(time(NULL)) ^ omp_get_thread_num());

        while(true) {
            size_t start;
            #pragma omp critical
            {
                start = current_index;
                current_index += PAGE_SIZE;
            }

            if (start >= size) {
                break;
            }

            size_t end = std::min(start + PAGE_SIZE, size);
            int repeat_count = rand() % 3 + 1;

            for (int r = 0; r < repeat_count; r++) {
                for (size_t i = start; i < end; i++) {
                    dst[i] = src[i];
                }
                clobber();
            }
        }
    }
}


double __attribute__ ((noinline)) run_test(double* v, int size, int num_threads, std::string name) {
    double sum = 0;

    LIKWID_MARKER_START(name.c_str());

    #pragma omp parallel for default(none) shared(v, size) reduction(+:sum) schedule(static) num_threads(num_threads)
    for (size_t i = 0; i < size; i++) {
        sum += v[i];
    }

    LIKWID_MARKER_STOP(name.c_str());

    return sum;
}

int main(int argc, char **argv) {

    size_t SIZE = 1l * 1024l * 1024l * 1024l;
    double* data_single = malloc_large_double(SIZE);
    double* data_multi_static = malloc_large_double(SIZE);
    double* data_multi_random = malloc_large_double(SIZE);
    double r;

    int num_threads = omp_get_max_threads();
    std::cout << "Num threads " << num_threads << "\n";

    LIKWID_MARKER_INIT;

    for (int rep = 0; rep < 1; rep++) {
        initialize_data_single_threaded(data_single, SIZE);
        r = run_test(data_single, SIZE, num_threads, "SINGLE");
        std::cout << "r = " << r << std::endl;

        copy_data_multithreaded_random(data_multi_random, data_single, SIZE, num_threads);
        r = run_test(data_multi_random, SIZE, num_threads, "RANDOM");
        std::cout << "r = " << r << std::endl;

        copy_data_multithreaded_static(data_multi_static, data_single, SIZE, num_threads);
        r = run_test(data_multi_random, SIZE, num_threads, "STATIC");
        std::cout << "r = " << r << std::endl;
    }

    free_large_double(data_single, SIZE);
    free_large_double(data_multi_static, SIZE);
    free_large_double(data_multi_random, SIZE);

    LIKWID_MARKER_CLOSE;
}
