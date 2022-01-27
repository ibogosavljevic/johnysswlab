#include "global.h"
#include <type_traits>

#include <immintrin.h>
#include <emmintrin.h>
#include <stdexcept>

// Source: https://www.geeksforgeeks.org/heap-sort/


template <typename T, int HeapSize, bool Branchless>
class heapify {
public:
    static void heapify_k(std::vector<T>& vec, int n, int i) {
        //static_assert(false, "Not implemented");
    }
};

template <typename T, int HeapSize>
class heapify<T, HeapSize, false> {
public:
    static void heapify_k(std::vector<T>& vec, int n, int i) {
        int largest = i;

        int start = HeapSize * i + 1;
        int end = std::min(start + HeapSize, n);

        for (int j = start; j < end; j++) {
            if (vec[j] > vec[largest]) {
                largest = j;
            }
        }

        if (largest != i) {
            std::swap(vec[i], vec[largest]);

            heapify_k(vec, n, largest);
        }
    }
};


template <int HeapSize>
class heapify<float, HeapSize, true> {
public:
    static void heapify_k(std::vector<float>& vec, int n, int i) {
        int start = HeapSize * i + 1;
        int end = std::min(start + HeapSize, n);

        __m128i v_largest;
        __m128i j_index;
        __m128 val_largest = _mm_load_ss(&vec[i]);     // val_largest = vec[i];
        v_largest = _mm_insert_epi32(v_largest, i, 0); // v_largest = i;
        j_index = _mm_insert_epi32(j_index, start, 0); // j_index = start;
        for (int j = start; j < end; j++) {
            __m128i j_index;
            j_index = _mm_insert_epi32(j_index, j, 0); // j_index = j;
            __m128 val_j = _mm_load_ss(&vec[j]);       // val_j = vec[j];
            __m128 compare = _mm_cmpgt_ss(val_j, val_largest); // compare = val_j > val_largest
            val_largest = _mm_blendv_ps(val_largest, val_j, compare); // val_largest = (val_j > val_largest) ? val_j : val_largest;
            v_largest = _mm_blendv_epi8(v_largest, j_index, _mm_castps_si128(compare)); // v_largest = (val_j > val_largest) ? j_index : largest;
        }

        int largest = _mm_extract_epi32(v_largest, 0);
        
        if (largest != i) {
            std::swap(vec[i], vec[largest]);

            heapify_k(vec, n, largest);
        }
    }
};


template <typename T, int HeapSize, bool Branchless>
void heapsort_k(std::vector<T>& vec) {

    int n = vec.size();

    for (int i = n / HeapSize - 1; i >= 0; i--) {
        //template <typename T, int HeapSize, bool Branchless>
        heapify<T, HeapSize, Branchless>::heapify_k(vec, n, i);
    }

    for (int i = n - 1; i > 0; i--) {
        std::swap(vec[0], vec[i]);

        //template <typename T, int HeapSize, bool Branchless>
        heapify<T, HeapSize, Branchless>::heapify_k(vec, i, 0);
    }
}

template <typename T, int HeapSize = 2>
void heapsort(std::vector<T>& vec) {
    heapsort_k<T, HeapSize, false>(vec);
}

template <typename T, int HeapSize = 2>
void heapsort_branchless(std::vector<T>& vec) {
    heapsort_k<T, HeapSize, true>(vec);
}