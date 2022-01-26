#include "global.h"
// Source: https://www.geeksforgeeks.org/quick-sort/

template <typename T>
int partition(std::vector<T>& vector, int low, int high) {
    T pivot = vector[high];

    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (vector[j] <= pivot) {
            i++;
            std::swap(vector[i], vector[j]);
        }
    }
    i = i + 1;
    std::swap(vector[i], vector[high]);
    return i;
}
/*
template <>
int partition(std::vector<float>& vector, int low, int high) {
    float* vector_i = &vector[low];
    float* vector_j = &vector[low];
    float* vector_end = &vector[high];

    __m128 pivot = _mm_load_ss(&vector[0] + high);
    while(true) {
        if (vector_j >= vector_end) break;
        __m128 vec_i = _mm_load_ss(vector_i);
        __m128 vec_j = _mm_load_ss(vector_j);

        __m128 compare = _mm_cmplt_ss(vec_j, pivot); // if (vec_j < pivot)
        __m128 new_vec_i = _mm_blendv_ps(vec_i, vec_j, compare);
        __m128 new_vec_j = _mm_blendv_ps(vec_j, vec_i, compare);

        int increment = _mm_extract_epi32(compare, 0) & 0x1;

        _mm_store_ss(vector_i, new_vec_i);
        _mm_store_ss(vector_j, new_vec_j);

        vector_i += increment;

        vector_j++;
    }

    std::swap(*vector_i, *vector_end);
    return (vector_i - &vector[0]);
}
*/

template <typename T>
void quicksort_internal(std::vector<T>& vector, int low, int high) {
    if (low < high) {
        int pi = partition(vector, low, high);

        quicksort_internal(vector, low, pi - 1);
        quicksort_internal(vector, pi + 1, high);
    }
}

template <typename T>
void quicksort(std::vector<T>& vector) {
    quicksort_internal(vector, 0, vector.size() - 1);
}
