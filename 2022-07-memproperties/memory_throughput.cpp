#include <vector>
#include "likwid.h"

#include <cassert>
#include <string>
#include <algorithm>
#include <numeric>
#include <iostream>

static constexpr int TEST_SIZE = 64 * 1024 * 1024;

#ifdef __AVX2__

#include <immintrin.h>

uint32_t hsum_epi32_avx(__m128i x)
{
    __m128i hi64  = _mm_unpackhi_epi64(x, x);           // 3-operand non-destructive AVX lets us save a byte without needing a movdqa
    __m128i sum64 = _mm_add_epi32(hi64, x);
    __m128i hi32  = _mm_shuffle_epi32(sum64, _MM_SHUFFLE(2, 3, 0, 1));    // Swap the low two elements
    __m128i sum32 = _mm_add_epi32(sum64, hi32);
    return _mm_cvtsi128_si32(sum32);       // movd
}

// only needs AVX2
uint32_t hsum_8x32(__m256i v)
{
    __m128i sum128 = _mm_add_epi32( 
                 _mm256_castsi256_si128(v),
                 _mm256_extracti128_si256(v, 1)); // silly GCC uses a longer AXV512VL instruction if AVX512 is enabled :/
    return hsum_epi32_avx(sum128);
}

#endif

template <int X>
int run_test(std::vector<int>& data, std::vector<int>& indexes, std::string name) {
    assert(data.size() == indexes.size());
    int len = data.size();
    int* data_ptr = &data[0];
    int* indexes_ptr = &indexes[0];
    int result;

    LIKWID_MARKER_START(name.c_str());

#ifdef __AVX2__

    __m256i sum = {0, 0, 0, 0 };

    for (int i = 0; i < len; i+=8) {
        const __m256i* indexes_2 = reinterpret_cast<const __m256i*>(indexes_ptr + i);
        __m256i index_reg = _mm256_loadu_si256(indexes_2);
        __m256i values = _mm256_i32gather_epi32(data_ptr, index_reg, 4);
        sum = _mm256_add_epi32(sum, values);
    }

    result = hsum_8x32(sum);
#else
    
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += data_ptr[indexes_ptr[i]];
    }

    result = sum;

#endif

    LIKWID_MARKER_STOP(name.c_str());

    /*double time = 0.0;
    int count = 0;
    int data_volume = (4 * 2 * len);
    double data_volume_mb = data_volume / (1024 * 1024);

    std::cout << "Region " << name << "\n";
    std::cout << "Calculated data volume = " << data_vol    ume / (1024 * 1024) << " MB\n";
    LIKWID_MARKER_GET(name.c_str(), nullptr, nullptr, &time, &count);
    std::cout << "Calculated throughput = " << data_volume_mb / time << " MB/s\n";*/

    return result;
}

template <typename T>
void generate_random_index_vector(T begin, T end, int size, int stride) {
    int current = 0;
    int start = 0;

    for (auto it = begin; it != end; ++it) {
        *it = current;
        current += stride;
        if (current >= size) {
            start++;
            current = start;
        }
    }
}

int main(int argc, char** argv) {

    LIKWID_MARKER_INIT;

    std::vector<int> data(TEST_SIZE);
    
    std::iota(data.begin(), data.end(), 0);
    std::random_shuffle(data.begin(), data.end());

    std::vector<int> indexes(TEST_SIZE);
    std::iota(indexes.begin(), indexes.end(), 0);
    int sum1 = run_test<1>(data, indexes, "Sequential");
    std::cout << sum1 << std::endl;

    std::random_shuffle(indexes.begin(), indexes.end());
    int sum2 = run_test<2>(data, indexes, "Random");
    std::cout << sum2 << std::endl;

    generate_random_index_vector(indexes.begin(), indexes.end(), TEST_SIZE, 4);
    int sum3 = run_test<3>(data, indexes, "Stride_4");
    std::cout << sum3 << std::endl;

    generate_random_index_vector(indexes.begin(), indexes.end(), TEST_SIZE, 16);
    int sum4 = run_test<4>(data, indexes, "Stride_16");
    std::cout << sum4 << std::endl;

    generate_random_index_vector(indexes.begin(), indexes.end(), TEST_SIZE, 32);
    int sum5 = run_test<5>(data, indexes, "Stride_32");
    std::cout << sum5 << std::endl;

    generate_random_index_vector(indexes.begin(), indexes.end(), TEST_SIZE, 64);
    int sum6 = run_test<6>(data, indexes, "Stride_64");
    std::cout << sum6 << std::endl;

    generate_random_index_vector(indexes.begin(), indexes.end(), TEST_SIZE, 128);
    int sum7 = run_test<7>(data, indexes, "Stride_128");
    std::cout << sum7 << std::endl;

    LIKWID_MARKER_CLOSE;


    return 0;
}
