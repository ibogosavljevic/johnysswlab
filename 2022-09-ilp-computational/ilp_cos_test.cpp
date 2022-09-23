#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "omp.h"
#include "likwid.h"

#ifdef __AVX2__
#include <immintrin.h>
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif


void initialize_data(std::vector<double> &values, int size)
{
    values.resize(size);
    for (int i = 0; i < size; i++)
    {
        values[i] = i + 1.0 / (static_cast<double>(i) + 0.1);
    }
}

double cos_scalar(double x) noexcept
{
    constexpr double tp = 1. / (2. * M_PI);
    x = x * tp;
    x = x - (double(.25) + std::floor(x + double(.25)));
    x = x * (double(16.) * (std::abs(x) - double(.5)));
    x = x + (double(.225) * x * (std::abs(x) - double(1.)));
    return x;
}

#ifdef __AVX2__

static constexpr int VECTOR_SIZE = 4;
using double_vec = __m256d;

double_vec load_vec(const double* addr) {
    return _mm256_loadu_pd(addr);
}

void store_vec(double* addr, double_vec value) {
    _mm256_storeu_pd(addr, value);
}

double_vec add_vec(double_vec a, double_vec b) {
    return _mm256_add_pd(a, b);
}

double_vec set_vec(double val) {
    return _mm256_set1_pd(val);
}


#elif defined(__ARM_NEON)

static constexpr int VECTOR_SIZE = 2;
using double_vec = float64x2_t;

double_vec load_vec(const double* addr) {
    return vld1q_f64(addr);
}

void store_vec(double* addr, double_vec value) {
    vst1q_f64(addr, value);
}

double_vec add_vec(double_vec a, double_vec b) {
    return vaddq_f64(a, b);
}

double_vec set_vec(double val) {
    return  vmovq_n_f64(val);
}


#else
#error Nothing defined
#endif


#ifdef __AVX2__

// X86 SPECIFIC CODE
__m256d _mm256_abs_pd(__m256d x) noexcept
{
    return _mm256_castsi256_pd(_mm256_srli_epi64(_mm256_slli_epi64(_mm256_castpd_si256(x), 1), 1));
}

__m256d cos_vector(__m256d x) noexcept
{
    constexpr double tp_scalar = 1. / (2. * M_PI);
    __m256d tmp;

    // x = x * tp;
    const __m256d tp = _mm256_set1_pd(tp_scalar);
    x = _mm256_mul_pd(x, tp);

    // x = x - (double(.25) + std::floor(x + double(.25)));
    const __m256d v_25 = _mm256_set1_pd(0.25);
    tmp = _mm256_add_pd(x, v_25);

    tmp = _mm256_floor_pd(tmp);
    tmp = _mm256_add_pd(v_25, tmp);

    x = _mm256_sub_pd(x, tmp);

    // x = x * (double(16.) * (std::abs(x) - double(.5)));
    const __m256d v_5 = _mm256_set1_pd(0.5);
    const __m256d v_16 = _mm256_set1_pd(16.0);

    tmp = _mm256_abs_pd(x);
    tmp = _mm256_sub_pd(tmp, v_5);
    tmp = _mm256_mul_pd(v_16, tmp);
    x = _mm256_mul_pd(x, tmp);

    // x = x + (double(.225) * x * (std::abs(x) - double(1.)));
    const __m256d v_225 = _mm256_set1_pd(0.225);
    const __m256d v_1 = _mm256_set1_pd(1.0);
    tmp = _mm256_abs_pd(x);
    tmp = _mm256_sub_pd(tmp, v_1);

    tmp = _mm256_mul_pd(x, tmp);
    tmp = _mm256_mul_pd(v_225, tmp);
    x = _mm256_add_pd(x, tmp);

    return x;
}

std::pair<__m256d, __m256d> cos_vector_interleaved(__m256d x1, __m256d x2) noexcept
{
    constexpr double tp_scalar = 1. / (2. * M_PI);
    __m256d tmp1;
    __m256d tmp2;

    // x = x * tp;
    const __m256d tp = _mm256_set1_pd(tp_scalar);
    x1 = _mm256_mul_pd(x1, tp);
    x2 = _mm256_mul_pd(x2, tp);

    // x = x - (double(.25) + std::floor(x + double(.25)));
    const __m256d v_25 = _mm256_set1_pd(0.25);
    tmp1 = _mm256_add_pd(x1, v_25);
    tmp2 = _mm256_add_pd(x2, v_25);

    tmp1 = _mm256_floor_pd(tmp1);
    tmp2 = _mm256_floor_pd(tmp2);

    tmp1 = _mm256_add_pd(v_25, tmp1);
    tmp2 = _mm256_add_pd(v_25, tmp2);

    x1 = _mm256_sub_pd(x1, tmp1);
    x2 = _mm256_sub_pd(x2, tmp2);

    // x = x * (double(16.) * (std::abs(x) - double(.5)));
    const __m256d v_5 = _mm256_set1_pd(0.5);
    const __m256d v_16 = _mm256_set1_pd(16.0);

    tmp1 = _mm256_abs_pd(x1);
    tmp2 = _mm256_abs_pd(x2);

    tmp1 = _mm256_sub_pd(tmp1, v_5);
    tmp2 = _mm256_sub_pd(tmp2, v_5);

    tmp1 = _mm256_mul_pd(v_16, tmp1);
    tmp2 = _mm256_mul_pd(v_16, tmp2);

    x1 = _mm256_mul_pd(x1, tmp1);
    x2 = _mm256_mul_pd(x2, tmp2);

    // x = x + (double(.225) * x * (std::abs(x) - double(1.)));
    const __m256d v_225 = _mm256_set1_pd(0.225);
    const __m256d v_1 = _mm256_set1_pd(1.0);
    tmp1 = _mm256_abs_pd(x1);
    tmp2 = _mm256_abs_pd(x2);

    tmp1 = _mm256_sub_pd(tmp1, v_1);
    tmp2 = _mm256_sub_pd(tmp2, v_1);

    tmp1 = _mm256_mul_pd(x1, tmp1);
    tmp2 = _mm256_mul_pd(x2, tmp2);

    tmp1 = _mm256_mul_pd(v_225, tmp1);
    tmp2 = _mm256_mul_pd(v_225, tmp2);

    x1 = _mm256_add_pd(x1, tmp1);
    x2 = _mm256_add_pd(x2, tmp2);

    return {x1, x2};
}

#elif defined(__ARM_NEON)

float64x2_t cos_vector(float64x2_t x) noexcept
{
    constexpr double tp_scalar = 1. / (2. * M_PI);
    float64x2_t tmp;

    // x = x * tp;
    const float64x2_t tp =  vmovq_n_f64(tp_scalar);
    x = vmulq_f64(x, tp);

    // x = x - (double(.25) + std::floor(x + double(.25)));
    const float64x2_t v_25 =  vmovq_n_f64(0.25);
    tmp = vaddq_f64(x, v_25);

    tmp = vrndmq_f64(tmp);
    tmp = vaddq_f64(v_25, tmp);

    x = vsubq_f64(x, tmp);

    // x = x * (double(16.) * (std::abs(x) - double(.5)));
    const float64x2_t v_5 =  vmovq_n_f64(0.5);
    const float64x2_t v_16 =  vmovq_n_f64(16.0);

    tmp = vabsq_f64(x);
    tmp = vsubq_f64(tmp, v_5);
    tmp = vmulq_f64(v_16, tmp);
    x = vmulq_f64(x, tmp);

    // x = x + (double(.225) * x * (std::abs(x) - double(1.)));
    const float64x2_t v_225 =  vmovq_n_f64(0.225);
    const float64x2_t v_1 =  vmovq_n_f64(1.0);
    tmp = vabsq_f64(x);
    tmp = vsubq_f64(tmp, v_1);

    tmp = vmulq_f64(x, tmp);
    tmp = vmulq_f64(v_225, tmp);
    x = vaddq_f64(x, tmp);

    return x;
}

std::pair<float64x2_t, float64x2_t> cos_vector_interleaved(float64x2_t x1, float64x2_t x2) noexcept
{
    constexpr double tp_scalar = 1. / (2. * M_PI);
    float64x2_t tmp1;
    float64x2_t tmp2;

    // x = x * tp;
    const float64x2_t tp =  vmovq_n_f64(tp_scalar);
    x1 = vmulq_f64(x1, tp);
    x2 = vmulq_f64(x2, tp);

    // x = x - (double(.25) + std::floor(x + double(.25)));
    const float64x2_t v_25 =  vmovq_n_f64(0.25);
    tmp1 = vaddq_f64(x1, v_25);
    tmp2 = vaddq_f64(x2, v_25);

    tmp1 = vrndmq_f64(tmp1);
    tmp2 = vrndmq_f64(tmp2);

    tmp1 = vaddq_f64(v_25, tmp1);
    tmp2 = vaddq_f64(v_25, tmp2);

    x1 = vsubq_f64(x1, tmp1);
    x2 = vsubq_f64(x2, tmp2);

    // x = x * (double(16.) * (std::abs(x) - double(.5)));
    const float64x2_t v_5 =  vmovq_n_f64(0.5);
    const float64x2_t v_16 =  vmovq_n_f64(16.0);

    tmp1 = vabsq_f64(x1);
    tmp2 = vabsq_f64(x2);

    tmp1 = vsubq_f64(tmp1, v_5);
    tmp2 = vsubq_f64(tmp2, v_5);

    tmp1 = vmulq_f64(v_16, tmp1);
    tmp2 = vmulq_f64(v_16, tmp2);

    x1 = vmulq_f64(x1, tmp1);
    x2 = vmulq_f64(x2, tmp2);

    // x = x + (double(.225) * x * (std::abs(x) - double(1.)));
    const float64x2_t v_225 =  vmovq_n_f64(0.225);
    const float64x2_t v_1 =  vmovq_n_f64(1.0);
    tmp1 = vabsq_f64(x1);
    tmp2 = vabsq_f64(x2);

    tmp1 = vsubq_f64(tmp1, v_1);
    tmp2 = vsubq_f64(tmp2, v_1);

    tmp1 = vmulq_f64(x1, tmp1);
    tmp2 = vmulq_f64(x2, tmp2);

    tmp1 = vmulq_f64(v_225, tmp1);
    tmp2 = vmulq_f64(v_225, tmp2);

    x1 = vaddq_f64(x1, tmp1);
    x2 = vaddq_f64(x2, tmp2);

    return {x1, x2};
}


#else
#error "Unsupported architecture"
#endif

void print(const std::vector<double> &v)
{
    for (const auto &i : v)
    {
        std::cout << i << ", ";
    }
    std::cout << "\n";
}

double horizontal_add(const double_vec v)
{
    double vec[VECTOR_SIZE];

    store_vec(vec, v);
    double res = 0.0;
    for (int i = 0; i < VECTOR_SIZE; i++)
    {
        res += v[i];
    }

    return res;
}

void print(const double_vec v)
{
    double vec[VECTOR_SIZE];

    store_vec(vec, v);
    std::cout << "[ ";
    for (int i = 0; i < (VECTOR_SIZE - 1); i++) {
        std::cout << v[i] << ", ";
    }
    std::cout << v[VECTOR_SIZE - 1] << " ]";
}


// ARM SPECIFIC CODE

void __attribute__((noinline)) run_test(const std::vector<double> v)
{
    std::string name = "shortchain";

    int size = v.size();
    const double *v_ptr = &v[0];
    {
        double sum = 0;
        LIKWID_MARKER_START(name.c_str());
#pragma clang loop vectorize(enable)
        for (int i = 0; i < size; i++)
        {
            double r = cos_scalar(v_ptr[i]);
            sum += r;
        }
        LIKWID_MARKER_STOP(name.c_str());

        std::cout << "sum = " << sum << std::endl;
    }

    {
        std::string name2 = name + "_vec";
        double_vec sum_vec = set_vec(0.0);

        LIKWID_MARKER_START(name2.c_str());

        for (int i = 0; i < size; i += VECTOR_SIZE)
        {
            double_vec val = load_vec(v_ptr + i);
            val = cos_vector(val);
            sum_vec = add_vec(sum_vec, val);
        }

        LIKWID_MARKER_STOP(name2.c_str());

        std::cout << "sum_vec = " << horizontal_add(sum_vec) << std::endl;
    }

    {
        std::string name3 = name + "_vecinterleaved";

        double_vec sum_vec1 = set_vec(0.0);
        double_vec sum_vec2 = set_vec(0.0);
        std::pair<double_vec, double_vec> res;

        LIKWID_MARKER_START(name3.c_str());

        for (int i = 0; i < size; i += 2*VECTOR_SIZE)
        {
            double_vec val1 = load_vec(v_ptr + i);
            double_vec val2 = load_vec(v_ptr + i + VECTOR_SIZE);
            res = cos_vector_interleaved(val1, val2);
            sum_vec1 = add_vec(sum_vec1, res.first);
            sum_vec2 = add_vec(sum_vec2, res.second);
        }

        LIKWID_MARKER_STOP(name3.c_str());
        std::cout << "sum_vec_interleaved = " << horizontal_add(add_vec(sum_vec1, sum_vec2)) << std::endl;
    }

}

void __attribute__((noinline)) run_test_longchain(const std::vector<double> v, int cnt1, int cnt2)
{
    std::string name = "longchain_" + std::to_string(cnt1) + "_" + std::to_string(cnt2);

    int size = v.size();
    const double *v_ptr = &v[0];

    double sum = 0;

    LIKWID_MARKER_START(name.c_str());
    for (int i = 0; i < cnt1; ++i)
    {
        double current_val = v_ptr[i];
        for (int j = 0; j < cnt2; ++j)
        {
            current_val = cos_scalar(current_val);
        }

        sum += current_val;
    }
    LIKWID_MARKER_STOP(name.c_str());

    std::cout << "sum = " << sum << std::endl;

    std::string name2 = name + "_vec";
    double_vec sum_vec = set_vec(0.0);

    LIKWID_MARKER_START(name2.c_str());
    for (int i = 0; i < cnt1; i += VECTOR_SIZE)
    {
        double_vec current_val = load_vec(v_ptr + i);
        for (int j = 0; j < cnt2; ++j)
        {
            current_val = cos_vector(current_val);
        }

        sum_vec += current_val;
    }

    LIKWID_MARKER_STOP(name2.c_str());

    std::cout << "sum_vec = " << horizontal_add(sum_vec) << std::endl;

    std::string name3 = name + "_vecinterleaved";

    double_vec sum_vec1 = set_vec(0.0);
    double_vec sum_vec2 = set_vec(0.0);
    std::pair<double_vec, double_vec> res;

    LIKWID_MARKER_START(name3.c_str());
    for (int i = 0; i < cnt1; i += 2*VECTOR_SIZE)
    {
        double_vec val1 = load_vec(v_ptr + i);
        double_vec val2 = load_vec(v_ptr + i + VECTOR_SIZE);
        for (int j = 0; j < cnt2; ++j)
        {
            auto r = cos_vector_interleaved(val1, val2);
            val1 = r.first;
            val2 = r.second;
        }

        sum_vec1 = add_vec(sum_vec1, val1);
        sum_vec2 = add_vec(sum_vec2, val2);
    }

    LIKWID_MARKER_STOP(name3.c_str());

    std::cout << "sum_vec_interleaved = " << horizontal_add(add_vec(sum_vec1, sum_vec2)) << std::endl;
}

int main(int argc, char **argv)
{

    int SIZE = 128 * 1024 * 1024;
    std::vector<double> data_single;
    double r;

    LIKWID_MARKER_INIT;

    initialize_data(data_single, SIZE);

    run_test(data_single);
    
    const int SIZE2 = 60 * 1024 * 1024;
    for (int i = 1; i < 50; i++) {
        
        run_test_longchain(data_single, SIZE2 / i, i);
    }

    int largest_index = 1024 * 1024;
    run_test_longchain(data_single, SIZE2/largest_index, largest_index);

    LIKWID_MARKER_CLOSE;
}
