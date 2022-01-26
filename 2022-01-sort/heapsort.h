#include "global.h"

// Source: https://www.geeksforgeeks.org/heap-sort/

template <typename T>
void heapify(std::vector<T>& vec, int n, int i) {
    int largest = i;
    int l = 2 * i + 1;
    int r = 2 * i + 2;

    if (l < n && vec[l] > vec[largest])
        largest = l;

    if (r < n && vec[r] > vec[largest])
        largest = r;

    if (largest != i) {
        std::swap(vec[i], vec[largest]);

        heapify(vec, n, largest);
    }
}

template <typename T>
void heapsort(std::vector<T>& vec) {

    int n = vec.size();

    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(vec, n, i);

    for (int i = n - 1; i > 0; i--) {
        std::swap(vec[0], vec[i]);

        heapify(vec, i, 0);
    }
}


template <typename T, int k>
void heapify_k(std::vector<T>& vec, int n, int i) {
    int largest = i;

    int start = k * i + 1;
    int end = std::min(start + k, n);

    for (int j = start; j < end; j++) {
        if (vec[j] > vec[largest]) {
            largest = j;
        }
    }

    if (largest != i) {
        std::swap(vec[i], vec[largest]);

        heapify_k<T, k>(vec, n, largest);
    }
}

#include <immintrin.h>
#include <emmintrin.h>
/*
template <>
void heapify2(std::vector<float>& vec, int n, int i) {
    int start = 4 * i + 1;
    int end = std::min(start + 4, n);

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
        //if (vec[j] > vec[largest]) {
        //    largest = j;
        //}
    }

    int largest = _mm_extract_epi32(v_largest, 0);
    
    if (largest != i) {
        std::swap(vec[i], vec[largest]);

        heapify2(vec, n, largest);
    }
}*/



/*__asm__ (
    "ucomiss %[vec_j], %[vec_largest];"
    "cmovbe %[j], %[largest];"
    : [largest] "+&r"(largest)
    : [vec_j] "m"(vec_j), [vec_largest] "x"(vec_largest), [j] "r" (j) 
    : "cc"
);*/


template <typename T, int k = 4>
void heapsort_k(std::vector<T>& vec) {

    int n = vec.size();

    for (int i = n / k - 1; i >= 0; i--)
        heapify_k<T, k>(vec, n, i);

    for (int i = n - 1; i > 0; i--) {
        std::swap(vec[0], vec[i]);

        heapify_k<T, k>(vec, i, 0);
    }
}