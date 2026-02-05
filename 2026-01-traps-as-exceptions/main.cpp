#include <cmath>
#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <random>
#include <vector>
#include "likwid.h"
#include <fenv.h>
#include <signal.h>
#include <setjmp.h>

#include <immintrin.h>
static constexpr size_t LANE_COUNT = 4;

void generate_non_negative_doubles(double* p, size_t n) {
    if (!p) return; // safeguard against null pointer

    std::random_device rd;  // non-deterministic seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_real_distribution<double> dist(0.0, 1.0); // range [0,1)

    for (size_t i = 0; i < n; ++i) {
        p[i] = dist(gen);
    }
}

__attribute__((noinline))
void clobber() {
  asm volatile("" : : : "memory");
}

size_t calculate_square_roots_scalar0(double* out, double * in, size_t n) {
    LIKWID_MARKER_START(__FUNCTION__);

    for (size_t i = 0; i < n; i++) {
        out[i] = std::sqrt(in[i]);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);


    return 0;
}

size_t calculate_square_roots_scalar1_count_nan(double* out, double * in, size_t n) {
    uint64_t total_nans = 0;

    LIKWID_MARKER_START(__FUNCTION__);

    for (size_t i = 0; i < n; i++) {
        double r = std::sqrt(in[i]);
        out[i] = r;
        total_nans += std::isnan(r);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);

    return total_nans;
}

#pragma STDC FENV_ACCESS ON
size_t calculate_square_roots_scalar2_sticky_nan(double* out, double * in, size_t n) {
    uint64_t total_nans = 0;
    static constexpr size_t SECTION_SIZE = 32;

    LIKWID_MARKER_START(__FUNCTION__);

    feclearexcept(FE_INVALID);

    for (size_t ii = 0; ii < n; ii+=SECTION_SIZE) {
        size_t i_end = std::min(ii + SECTION_SIZE, n);
        for (size_t i = ii; i < i_end; i++) {
            double r = std::sqrt(in[i]);
            out[i] = r;
        }

        if (fetestexcept(FE_INVALID)) {
            for (size_t i = ii; i < i_end; i++) {
                if (in[i] < 0) {
                    total_nans++;
                }
            }
            feclearexcept(FE_INVALID);
        }
    }


    LIKWID_MARKER_STOP(__FUNCTION__);

    return total_nans;
}
#pragma STDC FENV_ACCESS OFF

size_t calculate_square_roots_vec0(double* out, double * in, size_t n) {
    const size_t n_vector_end = n / LANE_COUNT * LANE_COUNT;

    LIKWID_MARKER_START(__FUNCTION__);

    for (size_t i = 0; i < n_vector_end; i+=LANE_COUNT) {
        __m256d r = _mm256_sqrt_pd(_mm256_loadu_pd(in + i));
        _mm256_storeu_pd(out + i, r);
    }

    for (size_t i = n_vector_end; i < n; i++) {
        out[i] = std::sqrt(in[i]);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);


    return 0;
}

size_t calculate_square_roots_vec_count_nan1(double* out, double * in, size_t n) {
    const size_t n_vector_end = n / LANE_COUNT * LANE_COUNT;
    uint64_t total_nans = 0;

    LIKWID_MARKER_START(__FUNCTION__);
    
    __m256i is_nans = _mm256_set1_epi64x(0);
    for (size_t i = 0; i < n_vector_end; i+=LANE_COUNT) {
        __m256d r = _mm256_sqrt_pd(_mm256_loadu_pd(in + i));
        _mm256_storeu_pd(out + i, r);
        __m256d is_nan = _mm256_cmp_pd(r, r, _CMP_UNORD_Q);
        is_nans = _mm256_sub_epi64(is_nans, _mm256_castpd_si256(is_nan));
    }

    alignas(32) uint64_t tmp[LANE_COUNT];
    _mm256_store_si256((__m256i*) tmp, is_nans);
    for (size_t i = 0; i < LANE_COUNT; i++) {
        total_nans += tmp[i];
    }

    for (size_t i = n_vector_end; i < n; i++) {
        double r = std::sqrt(in[i]);
        out[i] = r;
        total_nans += std::isnan(r);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);

    return total_nans;
}

#pragma STDC FENV_ACCESS ON
size_t calculate_square_roots_vec_sticky_nan(double* out, double * in, size_t n) {
    static constexpr size_t SECTION_SIZE = 8U * LANE_COUNT; 
    const size_t n_vector_end = n / SECTION_SIZE * SECTION_SIZE;
    uint64_t total_nans = 0;

    LIKWID_MARKER_START(__FUNCTION__);
    
    __m256i is_nans = _mm256_set1_epi64x(0);
    for (size_t ii = 0; ii < n_vector_end; ii+=SECTION_SIZE) {
        const size_t i_end = ii + SECTION_SIZE;
        for (size_t i = ii; i < i_end; i+=LANE_COUNT) {
            __m256d r = _mm256_sqrt_pd(_mm256_loadu_pd(in + i));
            _mm256_storeu_pd(out + i, r);
        }

        if (fetestexcept(FE_INVALID)) {
            for (size_t i = ii; i < i_end; i+=LANE_COUNT) {
                __m256d out_val = _mm256_loadu_pd(out + i);
                __m256d is_nan = _mm256_cmp_pd(out_val, out_val, _CMP_UNORD_Q);
                is_nans = _mm256_sub_epi64(is_nans, _mm256_castpd_si256(is_nan));
            }
            feclearexcept(FE_INVALID);
        }

    }


    alignas(32) uint64_t tmp[LANE_COUNT];
    _mm256_store_si256((__m256i*) tmp, is_nans);
    for (size_t i = 0; i < LANE_COUNT; i++) {
        total_nans += tmp[i];
    }

    for (size_t i = n_vector_end; i < n; i++) {
        double r = std::sqrt(in[i]);
        out[i] = r;
        total_nans += std::isnan(r);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);

    return total_nans;
}
#pragma STDC FENV_ACCESS OFF

//---------------------------------------------------------------------------------------

static sigjmp_buf fpe_env;

void fpe_handler(int sig, siginfo_t *info, void *ucontext)
{
    (void)sig; (void)info; (void)ucontext;

    // Clear sticky FP flags, otherwise retrap
    feclearexcept(FE_ALL_EXCEPT);
    feenableexcept(FE_INVALID);

    siglongjmp(fpe_env, 1);
}

#pragma STDC FENV_ACCESS ON
size_t calculate_square_roots_vec_count_nan_longjmp(double* out, double * in, size_t n) {
    const size_t n_vector_end = n / LANE_COUNT * LANE_COUNT;
    uint64_t total_nans = 0;

    // Save FP env
    fenv_t old_env;
    fegetenv(&old_env);

    feenableexcept(FE_INVALID);

    // Install SIGFPE handler
    struct sigaction act{}, oldact{};
    act.sa_sigaction = fpe_handler;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction(SIGFPE, &act, &oldact);

    LIKWID_MARKER_START(__FUNCTION__);

    for (size_t i = 0; i < n_vector_end; i += LANE_COUNT) {
        if (sigsetjmp(fpe_env, 1) == 0) {
            // normal path
            __m256d r =
                _mm256_sqrt_pd(_mm256_loadu_pd(in + i));
            _mm256_storeu_pd(out + i, r);
        } else {
            // SIGFPE recovery path
            for (size_t j = i; j < i + LANE_COUNT; j++) {
                total_nans += (in[j] < 0.0);
            }
        }
    }

    // Scalar tail
    for (size_t i = n_vector_end; i < n; i++) {
        if (in[i] >= 0.0) {
            out[i] = std::sqrt(in[i]);
        } else {
            total_nans++;
        }
    }

    LIKWID_MARKER_STOP(__FUNCTION__);

    // Restore state
    sigaction(SIGFPE, &oldact, nullptr);
    fesetenv(&old_env);

    return total_nans;
}
#pragma STDC FENV_ACCESS OFF


void throw_handler(int sig, siginfo_t *info, void *ucontext) {
    std::cout << "IVICA111\n";
    feclearexcept(FE_ALL_EXCEPT);
    feenableexcept(FE_INVALID);
    throw std::runtime_error("NaN! detected");
}


#pragma STDC FENV_ACCESS ON
size_t calculate_square_roots_vec_count_nan_cpp_exceptions(double* out, double * in, size_t n) {
    const size_t n_vector_end = n / LANE_COUNT * LANE_COUNT;
    uint64_t total_nans = 0;

    fenv_t old_env;
    fegetenv(&old_env);

    feenableexcept(FE_INVALID);

    struct sigaction act = {{ 0 }}, oldact = {{ 0 }};
    act.sa_sigaction = throw_handler;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction(SIGFPE, &act, &oldact);
    std::cout << "old act "<< oldact.sa_handler << "\n";

    LIKWID_MARKER_START(__FUNCTION__);

    for (size_t i = 0; i < n_vector_end; i+=LANE_COUNT) {
        try {
            __m256d r = _mm256_sqrt_pd(_mm256_loadu_pd(in + i));
            _mm256_storeu_pd(out + i, r);
        } catch(std::runtime_error& e) {
            std::cout << "Caught for i = " << i << "\n";
            for (size_t j = i; j < i + LANE_COUNT; j++) {
                total_nans += (in[j] < 0.0);
            }
            std::cout << "total nans = " << total_nans << "\n";
        }
    }

    for (size_t i = n_vector_end; i < n; i++) {
        if (in[i] >= 0) {
            out[i] = std::sqrt(in[i]);
        } else {
            total_nans++;
        }
    }

    LIKWID_MARKER_STOP(__FUNCTION__);

    sigaction(SIGFPE, &oldact, nullptr);
    fesetenv(&old_env);

    return total_nans;
}
#pragma STDC FENV_ACCESS OFF

int main() {
    static constexpr size_t vector_size = 1000*1024+3;
    static constexpr size_t repeat_count = 1;

    LIKWID_MARKER_INIT;

    std::vector<double> in(vector_size);
    generate_non_negative_doubles(in.data(), vector_size);
    in[5] = -1;
    in[6] = -1;
    in[1001] = -1;
    in[vector_size - 1] = -1;
    std::vector<double> out_scalar0(vector_size),
                        out_scalar1(vector_size),
                        out_scalar2(vector_size),
                        out0(vector_size),
                        out1(vector_size),
                        out2(vector_size),
                        out3(vector_size),
                        out4(vector_size);

    size_t nan_scalar0, nan_scalar1, nan_scalar2, nan_scalar3, nan0, nan1, nan2, nan3, nan4;

    for (size_t i = 0; i < repeat_count; i++) {
        nan_scalar0 = calculate_square_roots_scalar0(out_scalar0.data(), in.data(), vector_size);
        clobber();
    }

    for (size_t i = 0; i < repeat_count; i++) {
        nan_scalar1 = calculate_square_roots_scalar1_count_nan(out_scalar0.data(), in.data(), vector_size);
        clobber();
    }

    for (size_t i = 0; i < repeat_count; i++) {
        nan_scalar2 = calculate_square_roots_scalar2_sticky_nan(out_scalar1.data(), in.data(), vector_size);
        clobber();
    }

    for (size_t i = 0; i < repeat_count; i++) {
        nan_scalar2 = calculate_square_roots_scalar1_count_nan(out_scalar2.data(), in.data(), vector_size);
        clobber();
    }

    for (size_t i = 0; i < repeat_count; i++) {
        nan0 = calculate_square_roots_vec0(out0.data(), in.data(), vector_size);
        clobber();
    }

    for (size_t i = 0; i < repeat_count; i++) {
        nan1 = calculate_square_roots_vec_count_nan1(out1.data(), in.data(), vector_size);
        clobber();
    }

    for (size_t i = 0; i < repeat_count; i++) {
        nan2 = calculate_square_roots_vec_sticky_nan(out2.data(), in.data(), vector_size);
        clobber();
    }

    for (size_t i = 0; i < repeat_count; i++) {
        nan3 = calculate_square_roots_vec_count_nan_longjmp(out3.data(), in.data(), vector_size);
        clobber();
    }

    for (size_t i = 0; i < repeat_count; i++) {
        nan4 = calculate_square_roots_vec_count_nan_cpp_exceptions(out4.data(), in.data(), vector_size);
        clobber();
    }

    std::cout << "nan_scalar0 = " << nan_scalar0 << ", nan_scalar1 = " << nan_scalar1 << ", nan_scalar2 = " << nan_scalar2 << "\n";
    std::cout << "nan0 = " << nan0 << ", nan1 = " << nan1 << ", nan2 = " << nan2 << ", nan3 = " << nan3 << ", nan4 = " << nan4 << "\n";

    LIKWID_MARKER_CLOSE;

    return 0;
}
