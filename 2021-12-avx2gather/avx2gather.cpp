#include <likwid.h>
#include <immintrin.h>
#include <string>
#include <cassert>
#include <iostream>

static void escape(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}

template<typename T>
T* alloc_alligned(int size) {
    return (T*) malloc(size * sizeof(T));
}

void gather_scalar(double* a, double *b, int* indexes, int len, int repeat_count, std::string name) {
    for (int r = 0; r < repeat_count; r++) {
        LIKWID_MARKER_START(name.c_str());
        #pragma clang loop vectorize(disable)
        for (int i = 0; i < len; i++) {
            b[i] = a[indexes[i]];
        }
        LIKWID_MARKER_STOP(name.c_str());
        escape(b);
    }
}


void gather_vector(double* a, double* b, int* indexes, int len, int repeat_count, std::string name) {
    for (int r = 0; r < repeat_count; r++) {
        LIKWID_MARKER_START(name.c_str());

        #pragma clang loop vectorize(enable)
        for (int i = 0; i < len; i++) {
            b[i] = a[indexes[i]];
        }
        LIKWID_MARKER_STOP(name.c_str());
        escape(b);
    }
}


void gather_avx2_intrinsics(double* a, double* b, int* indexes, int len, int repeat_count, std::string name) {
    

    for (int r = 0; r < repeat_count; r++) {
        LIKWID_MARKER_START(name.c_str());
        for (int i = 0; i < len; i+=4) {
            const __m128i* indexes_2 = reinterpret_cast<const __m128i*>(indexes + i);
            __m128i index_reg = _mm_load_si128(indexes_2);
            __m256d values = _mm256_i32gather_pd(a, index_reg, 8);
            _mm256_storeu_pd(b + i, values);
        }
        LIKWID_MARKER_STOP(name.c_str());
        escape(b);
    }

    
}

enum indexing_e {
    START = 0,
    STRIDED = 0,
    RANDOM = 1,
    END = 2
};

inline indexing_e operator++ (indexing_e const& d) {
    return static_cast<indexing_e>((static_cast<int>(d) + 1));
}

std::string to_string(const indexing_e& d) {
    switch (d) {
        case STRIDED: return "STRIDED";
        case RANDOM: return "RANDOM";
        default: return "UNKNOWN";
    }
}

void fill_index_array(indexing_e indexing_type, int* indexes, int length, int stride) {
    if (indexing_type == STRIDED) {
        int start = 0;
        int current = 0;
        for (int i = 0; i < length; i++) {
            indexes[i] = current;
            current += stride;
            if (current >= length) {
                start++;
                current = start;
            }
        }
    } else if (indexing_type == RANDOM) {
        for (int i = 0; i < length; i++) {
            indexes[i] = i;
        }

        for (int i = length - 1; i > 0; i--) {
            int rand_index = lrand48() % (i + 1);
            std::swap(indexes[i], indexes[rand_index]);
        }
    } else {
        assert(false && "Unreachable");
    }
}

void fill_rand_array(double* a, int length) {
    for (int i = 0; i < length; i++) {
        a[i] = drand48();
    }
}

bool assert_equal(double* lhs, double* rhs, int len) {
    for (int i = 0; i < len; i++) {
        assert(lhs[i] == rhs[i]);
    }

    return true;
}


int main(int argc, char** argv) {
    static constexpr int start_size = 1024;
    static constexpr int end_size = 64 * 1024 * 1024;
    static constexpr int size = end_size;
    static constexpr int start_stride = 1;
    static constexpr int end_stride = 8 * 1024;

    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;


    for (int i = start_size; i <= end_size; i*=2) {
        double* a = alloc_alligned<double>(i);
        double* b1 = alloc_alligned<double>(i);
        double* b2 = alloc_alligned<double>(i);
        int* indexes = alloc_alligned<int>(i);
        int repeat_count = (size / i);

        fill_rand_array(a, i);

        for (int stride = start_stride; stride <= end_stride; stride*=2) {
            fill_index_array(indexing_e::STRIDED, indexes, i, stride);

            std::cout << "Running STRIDED, size = " << i << ", stride = " << stride << std::endl;

            gather_scalar(a, b1, indexes, i, repeat_count, std::to_string(i) + "_SCALAR_STRIDED_" + std::to_string(stride));
            gather_vector(a, b2, indexes, i, repeat_count, std::to_string(i) + "_VECTOR_STRIDED_" + std::to_string(stride)); 
            assert_equal(b1, b2, i);
            gather_avx2_intrinsics(a, b2, indexes, i, repeat_count, std::to_string(i) + "_AVX2_STRIDED_" + std::to_string(stride)); 
            assert_equal(b1, b2, i);
        }

        fill_index_array(indexing_e::RANDOM, indexes, i, 0);

        std::cout << "Running RANDOM, size = " << i << std::endl;

        gather_scalar(a, b1, indexes, i, repeat_count, std::to_string(i) + "_SCALAR_RANDOM");
        gather_vector(a, b2, indexes, i, repeat_count, std::to_string(i) + "_VECTOR_RANDOM");
        assert_equal(b1, b2, i); 
        gather_avx2_intrinsics(a, b2, indexes, i, repeat_count, std::to_string(i) + "_AVX2_RANDOM"); 
        assert_equal(b1, b2, i);

        free(a);
        free(b1);
        free(b2);
        free(indexes);
    }

    LIKWID_MARKER_CLOSE;

    return 0;
}
