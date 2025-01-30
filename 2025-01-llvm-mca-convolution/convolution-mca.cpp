#include <random>
#include <iostream>
#include <cassert>

#include "likwid.h"

#define _mm256_alignr_ps(a,b,imm8) _mm256_castsi256_ps(_mm256_alignr_epi8(_mm256_castps_si256(a), _mm256_castps_si256(b), (imm8)*4))

void convolution_simple(float* out, float * in, size_t in_size, float* kernel, size_t kernel_size) {
    LIKWID_MARKER_START(__FUNCTION__);

    for (size_t i = 0; i < (in_size - kernel_size); i++) {
        out[i] = 0.0f;
        for (size_t k = 0; k < kernel_size; k++) {
            out[i] += in[i + k] * kernel[k];
        }
    }

    LIKWID_MARKER_STOP(__FUNCTION__);
}

void convolution_interchanged(float* out, float * in, size_t in_size, float* kernel, size_t kernel_size) {
    assert(kernel_size > 0);
    LIKWID_MARKER_START(__FUNCTION__);

    float kernel_0 = kernel[0];
    for (size_t i = 0; i < (in_size - kernel_size); i++) {
        out[i] = in[i] * kernel_0;
    }
    
    for (size_t k = 1; k < kernel_size; k++) {
        float kernel_k = kernel[k];
        for (size_t i = 0; i < (in_size - kernel_size); i++) {
            out[i] += in[i + k] * kernel_k;
        }
    }

    LIKWID_MARKER_STOP(__FUNCTION__);
}

#if defined(__AVX2__)

#include "immintrin.h"

void convolution_outerloop(float* out, float * in, size_t in_size, float* kernel, size_t kernel_size) {
    LIKWID_MARKER_START(__FUNCTION__);

    for (size_t i = 0; i < (in_size - kernel_size); i+=8) {
        __m256 out_v = _mm256_set1_ps(0.0f);

        for (size_t k = 0; k < kernel_size; k++) {
            __m256 in_v = _mm256_loadu_ps(in + i + k);
            __m256 kernel_v = _mm256_broadcast_ss(kernel + k);
            out_v = _mm256_add_ps(out_v, _mm256_mul_ps(in_v, kernel_v));
        }
        _mm256_storeu_ps(out + i, out_v);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);
}

void convolution_outerloop_5(float* out, float * in, size_t in_size, float* kernel) {
    LIKWID_MARKER_START(__FUNCTION__);

    __m256 const kernel_0 = _mm256_set1_ps(kernel[0]);
    __m256 const kernel_1 = _mm256_set1_ps(kernel[1]);
    __m256 const kernel_2 = _mm256_set1_ps(kernel[2]);
    __m256 const kernel_3 = _mm256_set1_ps(kernel[3]);
    __m256 const kernel_4 = _mm256_set1_ps(kernel[4]);

    for (size_t i = 0; i < (in_size - 5); i+=8) {
        float const * in_i = in + i;
        __m256 out_v = _mm256_mul_ps(_mm256_loadu_ps(in_i), kernel_0);

        out_v = _mm256_add_ps(out_v, _mm256_mul_ps(_mm256_loadu_ps(in_i + 1), kernel_1));
        out_v = _mm256_add_ps(out_v, _mm256_mul_ps(_mm256_loadu_ps(in_i + 2), kernel_2));
        out_v = _mm256_add_ps(out_v, _mm256_mul_ps(_mm256_loadu_ps(in_i + 3), kernel_3));
        out_v = _mm256_add_ps(out_v, _mm256_mul_ps(_mm256_loadu_ps(in_i + 4), kernel_4));

        _mm256_storeu_ps(out + i, out_v);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);
}

void convolution_outerloop_asm_5(float* out, float * in, size_t in_size, float* kernel) {
    LIKWID_MARKER_START(__FUNCTION__);

    __m256 const kernel_0 = _mm256_set1_ps(kernel[0]);
    __m256 const kernel_1 = _mm256_set1_ps(kernel[1]);
    __m256 const kernel_2 = _mm256_set1_ps(kernel[2]);
    __m256 const kernel_3 = _mm256_set1_ps(kernel[3]);
    __m256 const kernel_4 = _mm256_set1_ps(kernel[4]);

    for (size_t i = 0; i < (in_size - 5); i+=8) {
        __m256 out_v {};

        float const * in_i = in + i;
        __asm (
            "vmulps (%[in_i]), %[kernel_0], %[out_v]\n"
            "vmulps 4(%[in_i]), %[kernel_1], %%ymm1\n"
            "vmulps 8(%[in_i]), %[kernel_2], %%ymm2\n"
            "vmulps 12(%[in_i]), %[kernel_3], %%ymm3\n"
            "vmulps 16(%[in_i]), %[kernel_4], %%ymm4\n"
            "vaddps %[out_v], %%ymm1, %[out_v]\n"
            "vaddps %[out_v], %%ymm2, %[out_v]\n"
            "vaddps %[out_v], %%ymm3, %[out_v]\n"
            "vaddps %[out_v], %%ymm4, %[out_v]\n"
            : [out_v] "+x" (out_v)
            : [kernel_0] "x" (kernel_0), [kernel_1] "x" (kernel_1), [kernel_2] "x" (kernel_2), [kernel_3] "x" (kernel_3), [kernel_4] "x" (kernel_4), [in_i] "r" (in_i)
            : "ymm1", "ymm2", "ymm3", "ymm4"
        );
        _mm256_storeu_ps(out + i, out_v);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);
}

template <int end>
__m256 convolution_outerloop2_innerloop(size_t kk, float * kernel, __m256 inval_0, __m256 inval_1, __m256 out) {
    __m256 result = out;
    //#pragma GCC unroll 4    
    #pragma unroll
    for (size_t k = kk, offset = 0; k < kk + end; k++, offset++) {
        __m256 in_v = _mm256_alignr_ps(inval_1, inval_0, offset);
        __m256 kernel_v = _mm256_broadcast_ss(kernel + k);
        result = _mm256_add_ps(result, _mm256_mul_ps(in_v, kernel_v));
    }
    return result;
}

void convolution_outerloop2_5(float* out, float * in, size_t in_size, float* kernel) {

    __m256 const kernel_0 = _mm256_set1_ps(kernel[0]);
    __m256 const kernel_1 = _mm256_set1_ps(kernel[1]);
    __m256 const kernel_2 = _mm256_set1_ps(kernel[2]);
    __m256 const kernel_3 = _mm256_set1_ps(kernel[3]);
    __m256 const kernel_4 = _mm256_set1_ps(kernel[4]);

    LIKWID_MARKER_START(__FUNCTION__);
    
    for (size_t i = 0; i < (in_size - 5); i+=8) {
        
        __m256 in_0 = _mm256_loadu_ps(in + i);
        __m256 in_4 = _mm256_loadu_ps(in + i + 4);
        
        __m256 in_1 = _mm256_alignr_ps(in_4, in_0, 1);
        __m256 in_2 = _mm256_alignr_ps(in_4, in_0, 2);
        __m256 in_3 = _mm256_alignr_ps(in_4, in_0, 3);

        __m256 out_v = _mm256_mul_ps(in_0, kernel_0);
        in_1 = _mm256_mul_ps(in_1, kernel_1);
        in_2 = _mm256_mul_ps(in_2, kernel_2);
        in_3 = _mm256_mul_ps(in_3, kernel_3);
        in_4 = _mm256_mul_ps(in_4, kernel_4);

        out_v = _mm256_add_ps(out_v, in_1);
        out_v = _mm256_add_ps(out_v, in_2);
        out_v = _mm256_add_ps(out_v, in_3);
        out_v = _mm256_add_ps(out_v, in_4);
        
        _mm256_storeu_ps(out + i, out_v);
    }


    LIKWID_MARKER_STOP(__FUNCTION__);
}

void convolution_outerloop2_asm_5(float* out, float * in, size_t in_size, float* kernel) {

    __m256 const kernel_0 = _mm256_set1_ps(kernel[0]);
    __m256 const kernel_1 = _mm256_set1_ps(kernel[1]);
    __m256 const kernel_2 = _mm256_set1_ps(kernel[2]);
    __m256 const kernel_3 = _mm256_set1_ps(kernel[3]);
    __m256 const kernel_4 = _mm256_set1_ps(kernel[4]);

    LIKWID_MARKER_START(__FUNCTION__);
    
    for (size_t i = 0; i < (in_size - 5); i+=8) {
        __m256 out_v{};

        asm(
            "vmovups (%[in], %[i], 4), %%ymm0\n"
            "vmovups 16(%[in], %[i], 4), %%ymm4\n"
            "vpalignr $4, %%ymm0, %%ymm4, %%ymm1\n"
            "vpalignr $8, %%ymm0, %%ymm4, %%ymm2\n"
            "vpalignr $12, %%ymm0, %%ymm4, %%ymm3\n"

            "vmulps %%ymm0, %[kernel_0], %[out_v]\n"
            "vmulps %%ymm1, %[kernel_1], %%ymm1\n"
            "vmulps %%ymm2, %[kernel_2], %%ymm2\n"
            "vmulps %%ymm3, %[kernel_3], %%ymm3\n"
            "vmulps %%ymm4, %[kernel_4], %%ymm4\n"
            "vaddps %[out_v], %%ymm1, %[out_v]\n"
            "vaddps %[out_v], %%ymm2, %[out_v]\n"
            "vaddps %[out_v], %%ymm3, %[out_v]\n"
            "vaddps %[out_v], %%ymm4, %[out_v]\n"
            : [out_v] "+x" (out_v)
            : [kernel_0] "x" (kernel_0), [kernel_1] "x" (kernel_1), [kernel_2] "x" (kernel_2), [kernel_3] "x" (kernel_3), [kernel_4] "x" (kernel_4), [in] "r" (in), [i] "r" (i)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4"
        );

        _mm256_storeu_ps(out + i, out_v);
    }


    LIKWID_MARKER_STOP(__FUNCTION__);
}


void convolution_outerloop2(float* out, float * in, size_t in_size, float* kernel, size_t kernel_size) {
    size_t const kernel_size_vec_end = kernel_size / 4 * 4;

    LIKWID_MARKER_START(__FUNCTION__);
    
    for (size_t i = 0; i < (in_size - kernel_size); i+=8) {
        __m256 out_v = _mm256_set1_ps(0.0f);

        __m256 inval_0;
        __m256 inval_1 = _mm256_loadu_ps(in + i);
        for (size_t kk = 0; kk < kernel_size_vec_end; kk+=4) {
            inval_0 = inval_1;
            inval_1 = _mm256_set_m128(_mm_loadu_ps(in + i + kk + 8), _mm256_extractf128_ps(inval_1, 1));

            out_v = convolution_outerloop2_innerloop<4>(kk, kernel, inval_0, inval_1, out_v);
        }

        switch(kernel_size - kernel_size_vec_end) {
            case 0: break;
            case 1: {
                inval_0 = inval_1;
                inval_1 = _mm256_set_m128(_mm_loadu_ps(in + i + kernel_size_vec_end + 8), _mm256_extractf128_ps(inval_1, 1));
                out_v = convolution_outerloop2_innerloop<1>(kernel_size_vec_end, kernel, inval_0, inval_1, out_v);
                break;
            }
            case 2: {
                inval_0 = inval_1;
                inval_1 = _mm256_set_m128(_mm_loadu_ps(in + i + kernel_size_vec_end + 8), _mm256_extractf128_ps(inval_1, 1));
                out_v = convolution_outerloop2_innerloop<2>(kernel_size_vec_end, kernel, inval_0, inval_1, out_v);
                break;
            }
            case 3: {
                inval_0 = inval_1;
                inval_1 = _mm256_set_m128(_mm_loadu_ps(in + i + kernel_size_vec_end + 8), _mm256_extractf128_ps(inval_1, 1));
                out_v = convolution_outerloop2_innerloop<3>(kernel_size_vec_end, kernel, inval_0, inval_1, out_v);
                break;
            }
            default:
                assert(false && "Unreachable");
        }

        _mm256_storeu_ps(out + i, out_v);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);
}

#elif defined(__ARM_NEON)

#include <arm_neon.h>

void convolution_outerloop(float* out, float * in, size_t in_size, float* kernel, size_t kernel_size) {
    LIKWID_MARKER_START(__FUNCTION__);

    for (size_t i = 0; i < (in_size - kernel_size); i+=4) {
        float32x4_t out_v = vdupq_n_f32(0.0f);

        for (size_t k = 0; k < kernel_size; k++) {
            float32x4_t in_v = vld1q_f32(in + i + k);
            float32x4_t kernel_v = vld1q_dup_f32(kernel + k);
            out_v = vaddq_f32(out_v, vmulq_f32(in_v, kernel_v));
        }
        vst1q_f32(out + i, out_v);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);
}

template <int end>
float32x4_t convolution_outerloop2_innerloop(size_t kk, float * kernel, float32x4_t inval_0, float32x4_t inval_1, float32x4_t out) {
    float32x4_t result = out;
    #pragma GCC unroll 4    
    for (size_t k = kk, offset = 0; k < kk + end; k++, offset++) {
        float32x4_t in_v = vextq_f32(inval_0, inval_1, offset);
        float32x4_t kernel_v = vld1q_dup_f32(kernel + k);
        result = vaddq_f32(result, vmulq_f32(in_v, kernel_v));
    }
    return result;
}

void convolution_outerloop2(float* out, float * in, size_t in_size, float* kernel, size_t kernel_size) {
    size_t const kernel_size_vec_end = kernel_size / 4 * 4;

    LIKWID_MARKER_START(__FUNCTION__);
    
    for (size_t i = 0; i < (in_size - kernel_size); i+=4) {
        float32x4_t out_v = vdupq_n_f32(0.0f);

        float32x4_t inval_0;
        float32x4_t inval_1 = vld1q_f32(in + i);
        for (size_t kk = 0; kk < kernel_size_vec_end; kk+=4) {
            inval_0 = inval_1;
            inval_1 = vld1q_f32(in + i + kk + 4);

            out_v = convolution_outerloop2_innerloop<4>(kk, kernel, inval_0, inval_1, out_v);
        }

        switch(kernel_size - kernel_size_vec_end) {
            case 0: break;
            case 1: {
                inval_0 = inval_1;
                inval_1 = vld1q_f32(in + i + kernel_size_vec_end + 4);
                out_v = convolution_outerloop2_innerloop<1>(kernel_size_vec_end, kernel, inval_0, inval_1, out_v);
                break;
            }
            case 2: {
                inval_0 = inval_1;
                inval_1 = vld1q_f32(in + i + kernel_size_vec_end + 4);
                out_v = convolution_outerloop2_innerloop<2>(kernel_size_vec_end, kernel, inval_0, inval_1, out_v);
                break;
            }
            case 3: {
                inval_0 = inval_1;
                inval_1 = vld1q_f32(in + i + kernel_size_vec_end + 4);
                out_v = convolution_outerloop2_innerloop<3>(kernel_size_vec_end, kernel, inval_0, inval_1, out_v);
                break;
            }
            default:
                assert(false && "Unreachable");
        }

        vst1q_f32(out + i, out_v);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);
}

void convolution_outerloop_5(float* out, float * in, size_t in_size, float* kernel) {
    LIKWID_MARKER_START(__FUNCTION__);

    float32x4_t const kernel_0 = vld1q_f32(kernel);
    float32x4_t const kernel_1 = vld1q_f32(kernel + 4);

    for (size_t i = 0; i < (in_size - 5); i+=4) {
        float const * in_i = in + i;
        float32x4_t in_0 = vld1q_f32(in_i);
        float32x4_t in_1 = vld1q_f32(in_i + 1);
        float32x4_t in_2 = vld1q_f32(in_i + 2);
        float32x4_t in_3 = vld1q_f32(in_i + 3);
        float32x4_t in_4 = vld1q_f32(in_i + 4);

        float32x4_t out_v = vmulq_laneq_f32(in_0, kernel_0, 0);

        out_v = vmlaq_laneq_f32(out_v, in_1, kernel_0, 1);
        out_v = vmlaq_laneq_f32(out_v, in_2, kernel_0, 2);
        out_v = vmlaq_laneq_f32(out_v, in_3, kernel_0, 3);
        out_v = vmlaq_laneq_f32(out_v, in_4, kernel_1, 0);

        vst1q_f32(out + i, out_v);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);
}

void convolution_outerloop_asm_5(float* out, float * in, size_t in_size, float* kernel) {
    LIKWID_MARKER_START(__FUNCTION__);

    float32x4_t const kernel_0 = vld1q_f32(kernel);
    float32x4_t const kernel_1 = vld1q_f32(kernel + 4);

    for (size_t i = 0; i < (in_size - 5); i+=4) {
        float32x4_t out_v {};

        float const * in_i = in + i;
        asm (
            "ldr q0, [%[in_i]]\n"
            "ldr q1, [%[in_i], #4]\n"
            "ldr q2, [%[in_i], #8]\n"
            "ldr q3, [%[in_i], #12]\n"
            "ldr q4, [%[in_i], #16]\n"
            "fmul %[out_v].4s, v0.4s, %[kernel_0].s[0]\n"
            "fmla %[out_v].4s, v1.4s, %[kernel_0].s[1]\n"
            "fmla %[out_v].4s, v2.4s, %[kernel_0].s[2]\n"
            "fmla %[out_v].4s, v3.4s, %[kernel_0].s[3]\n"
            "fmla %[out_v].4s, v4.4s, %[kernel_1].s[0]\n"
            : [out_v] "+x" (out_v)
            : [in_i] "r" (in_i), [kernel_0] "x" (kernel_0), [kernel_1] "x" (kernel_1)
            : "v0", "v1", "v2", "v3", "v4"
        );
        vst1q_f32(out + i, out_v);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);
}

void convolution_outerloop2_5(float* out, float * in, size_t in_size, float* kernel) {
    float32x4_t const kernel_0 = vld1q_f32(kernel);
    float32x4_t const kernel_1 = vld1q_f32(kernel + 4);

    LIKWID_MARKER_START(__FUNCTION__);
    
    for (size_t i = 0; i < (in_size - 5); i+=4) {
        float const * in_i = in + i;

        float32x4x2_t in = vld1q_f32_x2(in_i);
        float32x4_t in_1 = vextq_f32(in.val[0], in.val[1], 1);
        float32x4_t in_2 = vextq_f32(in.val[0], in.val[1], 2);
        float32x4_t in_3 = vextq_f32(in.val[0], in.val[1], 3);

        float32x4_t out_v = vmulq_laneq_f32(in.val[0], kernel_0, 0);

        out_v = vmlaq_laneq_f32(out_v, in_1, kernel_0, 1);
        out_v = vmlaq_laneq_f32(out_v, in_2, kernel_0, 2);
        out_v = vmlaq_laneq_f32(out_v, in_3, kernel_0, 3);
        out_v = vmlaq_laneq_f32(out_v, in.val[1], kernel_1, 0);

        vst1q_f32(out + i, out_v);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);
}

void convolution_outerloop2_asm_5(float* out, float * in, size_t in_size, float* kernel) {
    float32x4_t const kernel_0 = vld1q_f32(kernel);
    float32x4_t const kernel_1 = vld1q_f32(kernel + 4);

    LIKWID_MARKER_START(__FUNCTION__);
    
    for (size_t i = 0; i < (in_size - 5); i+=4) {
        float32x4_t out_v {};
        float const * in_i = in + i;

        asm(
            "ldp q0, q1, [%[in_i]]\n"
            "ext v11.16b, v0.16b, v1.16b, 4\n"
            "ext v12.16b, v0.16b, v1.16b, 8\n"
            "ext v13.16b, v0.16b, v1.16b, 12\n"
            "fmul %[out_v].4s, v0.4s, %[kernel_0].s[0]\n"
            "fmla %[out_v].4s, v11.4s, %[kernel_0].s[1]\n"
            "fmla %[out_v].4s, v12.4s, %[kernel_0].s[2]\n"
            "fmla %[out_v].4s, v13.4s, %[kernel_0].s[3]\n"
            "fmla %[out_v].4s, v1.4s, %[kernel_1].s[0]\n"
            : [out_v] "+x" (out_v)
            : [in_i] "r" (in_i), [kernel_0] "x" (kernel_0), [kernel_1] "x" (kernel_1)
            : "v0", "v1", "v11", "v12", "v13"
        );

        vst1q_f32(out + i, out_v);
    }

    LIKWID_MARKER_STOP(__FUNCTION__);
}


#else
#error Unsupported architecture
#endif

std::vector<float> generate_random_floats(size_t count, float min, float max) {
    // Create a random device and a random number generator
    std::random_device random_device;
    std::mt19937 generator(random_device());
    
    // Define a uniform real distribution within the range [min, max]
    std::uniform_real_distribution<float> distribution(min, max);

    // Generate the random numbers
    std::vector<float> random_numbers(count);
    for (auto& num : random_numbers) {
        num = distribution(generator);
    }

    return random_numbers;
}

bool compare_buffers(const std::vector<float>& v0, const std::vector<float>& v1, size_t count) {
    if (v0.size() < count || v1.size() < count) {
        std::cout << "Buffers too small\n";
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        if (std::abs(v0[i] - v1[i]) > 0.00001) {
            std::cout << "Difference at i " << i << ", v0 " << v0[i] << ", v1 " << v1[i] << "\n";
            return false;
        }
    }

    std::cout << "Same\n";
    return true;
}

int main() {
    size_t out_size = 1000000;
    size_t kernel_size = 5 + rand() % 10;
    size_t in_size = out_size + kernel_size;
    std::vector<float> kernel = generate_random_floats(kernel_size, 0.0f, 1.0f);
    std::vector<float> input = generate_random_floats(in_size, 0.0f, 1.0f);
    std::vector<float> out_simple(out_size + 10);
    std::vector<float> out_interchanged(out_size + 10);
    std::vector<float> out_outerloop(out_size + 10);
    std::vector<float> out_outerloop2(out_size + 10);
    std::vector<float> out_outerloop_asm_5(out_size + 10);
    std::vector<float> out_outerloop2_asm_5(out_size + 10);
    std::vector<float> out_outerloop_5(out_size + 10);
    std::vector<float> out_outerloop2_5(out_size + 10);


    std::cout << "Kernel size = " << kernel_size << std::endl;

    LIKWID_MARKER_INIT;

    for (size_t i = 0; i < 100; i++) {
        convolution_simple(out_simple.data(), input.data(), in_size, kernel.data(), kernel.size());
        convolution_interchanged(out_interchanged.data(), input.data(), in_size, kernel.data(), kernel.size());
        convolution_outerloop(out_outerloop.data(), input.data(), in_size, kernel.data(), kernel.size());
        convolution_outerloop2(out_outerloop2.data(), input.data(), in_size, kernel.data(), kernel.size());
    }

    for (size_t i = 0; i < 100; i++) {
        convolution_outerloop_5(out_outerloop_5.data(), input.data(), in_size, kernel.data());
        convolution_outerloop2_5(out_outerloop2_5.data(), input.data(), in_size, kernel.data());
        convolution_outerloop_asm_5(out_outerloop_asm_5.data(), input.data(), in_size, kernel.data());
        convolution_outerloop2_asm_5(out_outerloop2_asm_5.data(), input.data(), in_size, kernel.data());
    }


    LIKWID_MARKER_CLOSE;

    compare_buffers(out_simple, out_interchanged, out_size);
    compare_buffers(out_simple, out_outerloop, out_size);
    compare_buffers(out_simple, out_outerloop2, out_size);

    convolution_interchanged(out_interchanged.data(), input.data(), in_size, kernel.data(), 5);
    compare_buffers(out_interchanged, out_outerloop_asm_5, out_size);
    compare_buffers(out_interchanged, out_outerloop2_asm_5, out_size);
    compare_buffers(out_interchanged, out_outerloop_5, out_size);
    compare_buffers(out_interchanged, out_outerloop2_5, out_size);

    return 0;
}