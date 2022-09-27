#include <iostream>
#include <string>
#include <vector>
#include "omp.h"
#include "likwid.h"

static void clobber() {
  asm volatile("" : : : "memory");
}

void initialize_data_single_threaded(std::vector<double>& values, size_t size) {
    values.resize(size);
    for (size_t i = 0; i < size; i++) {
        values[i] = 1.0 / (1.0 + static_cast<double>(i));
    }
}

void copy_data_multithreaded_static(std::vector<double>&dst, const std::vector<double>&src, int num_cores) {
    size_t size = src.size();
    dst.resize(size);

    double * dst_ptr = &dst[0];
    double const * src_ptr = &src[0];

    #pragma omp parallel for default(none) shared(src_ptr, dst_ptr, size) schedule(static) num_threads(num_cores)
    for (size_t i = 0; i < size; i++) {
        dst_ptr[i] = src_ptr[i];
    }
}

void copy_data_multithreaded_random(std::vector<double>&dst, const std::vector<double>&src, int num_cores) {
    size_t size = src.size();
    dst.resize(size);

    double * dst_ptr = &dst[0];
    double const * src_ptr = &src[0];

    static constexpr int PAGE_SIZE = 4 * 1024 / sizeof(double);
    size_t current_index = 0;

    #pragma omp parallel default(none) shared(src_ptr, dst_ptr, size, current_index) num_threads(num_cores)
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
                    dst_ptr[i] = src_ptr[i];
                }
                clobber();
            }
        }
    }
}


double __attribute__ ((noinline)) run_test(const std::vector<double> v, int num_threads, std::string name) {
    size_t size = v.size();
    const double* v_ptr = &v[0];

    double sum = 0;

    LIKWID_MARKER_START(name.c_str());

    #pragma omp parallel for default(none) shared(v_ptr, size) reduction(+:sum) schedule(static) num_threads(num_threads)
    for (size_t i = 0; i < size; i++) {
        sum += v_ptr[i];
    }

    LIKWID_MARKER_STOP(name.c_str());

    return sum;
}

int main(int argc, char **argv) {

    size_t SIZE = 1l * 1024l * 1024l * 1024l;
    std::vector<double> data_single;
    std::vector<double> data_multi_static;
    std::vector<double> data_multi_random;
    double r;

    int num_threads = omp_get_max_threads();
    std::cout << "Num threads " << num_threads << "\n";

    LIKWID_MARKER_INIT;

    for (int rep = 0; rep < 1; rep++) {
    initialize_data_single_threaded(data_single, SIZE);
    r = run_test(data_single, num_threads, "SINGLE");
    std::cout << "r = " << r << std::endl;

    copy_data_multithreaded_random(data_multi_random, data_single, 4);
    r = run_test(data_multi_random, num_threads, "RANDOM");
    std::cout << "r = " << r << std::endl;

    copy_data_multithreaded_static(data_multi_static, data_single, 4);
    r = run_test(data_multi_random, num_threads, "STATIC");
    std::cout << "r = " << r << std::endl;
  }


    LIKWID_MARKER_CLOSE;



}
