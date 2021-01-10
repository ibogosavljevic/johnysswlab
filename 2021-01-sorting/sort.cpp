#include <algorithm>
#include <cassert>
#include <iostream>
#include <utility>
#include <vector>
#include "measure_time.h"
#include "utils.h"

using std::size_t;

template <typename T>
void print_array(T array[], int len) {
    for (int i = 0; i < len; i++) {
        std::cout << array[i] << ", ";
    }
    std::cout << std::endl;
}

template <typename T>
void quicksort(T array[], int left, int right) {
    if (left >= right) {
        return;
    }

    int index_left = left;
    int index_right = right;
    int pivot_index = (left + right) / 2;
    T pivot = array[pivot_index];

    while (index_left < index_right) {
        if (array[index_left] <= pivot) {
            index_left++;
            continue;
        }

        if (array[index_right] > pivot) {
            index_right--;
            continue;
        }

        std::swap(array[index_left], array[index_right]);

        if (index_left == pivot_index) {
            pivot_index = index_right;
        }
        if (index_right == pivot_index) {
            pivot_index = index_left;
        }
    }

    if (array[index_right] > pivot) {
        std::swap(array[pivot_index], array[index_right - 1]);
        pivot_index = index_right - 1;
    } else {
        std::swap(array[pivot_index], array[index_right]);
        pivot_index = index_right;
    }

    quicksort(array, left, pivot_index - 1);
    quicksort(array, pivot_index + 1, right);
}

// Taken from: https://www.programiz.com/dsa/heap-sort
template <typename T>
void heapify(T arr[], int n, int i) {
    // Find largest among root, left child and right child
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && arr[left] > arr[largest])
        largest = left;

    if (right < n && arr[right] > arr[largest])
        largest = right;

    // Swap and continue heapifying if root is not largest
    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        heapify(arr, n, largest);
    }
}

template <typename T>
void heapsort(T arr[], int n) {
    // Build max heap
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    // Heap sort
    for (int i = n - 1; i >= 0; i--) {
        std::swap(arr[0], arr[i]);

        // Heapify root element to get highest element at root again
        heapify(arr, i, 0);
    }
}

static constexpr int ARR_LEN = 1024 * 1024 * 10;

template <typename T, int len = 4>
void selection_sort(T a[]) {
    for (int i = 0; i < len; i++) {
        T min = a[i];
        T min_index = i;
        for (int j = i + 1; j < len; j++) {
            if (a[j] < min) {
                min = a[j];
                min_index = j;
            }
        }
        std::swap(a[i], a[min_index]);
    }
}

template <typename T, int len = 4>
void insertion_sort(T a[]) {
    for (int i = 1; i < len; i++) {
        for (int j = i; j > 0; j--) {
            if (a[j] >= a[j - 1]) {
                break;
            }
            std::swap(a[j], a[j - 1]);
        }
    }
}

// Taken from:
// https://github.com/JMarianczuk/SmallSorters/blob/a119c50d8e69e477f5af3d27d2a412b013a9680e/conditional_swap/ConditionalSwapX86.h
static inline void swap_if_greater(int& left, int& right) {
    /*    int tmp = left;
        __asm__(
            "cmp %[left],%[right]\n\t"
            "cmovb %[right],%[left]\n\t"
            "cmovb %[tmp],%[right]\n\t"
            : [ left ] "=&r"(left), [ right ] "=&r"(right)
            : "0"(left), "1"(right), [ tmp ] "r"(tmp)
            : "cc");*/
    if (left > right) {
        std::swap(left, right);
    }
}

template <typename T>
void sorting_network_sort4(T arr[]) {
    T a0_1 = std::min(arr[0], arr[2]);
    T a2_1 = std::max(arr[0], arr[2]);
    T a1_1 = std::min(arr[1], arr[3]);
    T a3_1 = std::max(arr[1], arr[3]);
    arr[0] = std::min(a0_1, a1_1);
    T a1_2 = std::max(a0_1, a1_1);
    T a2_2 = std::min(a2_1, a3_1);
    arr[3] = std::max(a2_1, a3_1);
    arr[1] = std::min(a1_2, a2_2);
    arr[2] = std::max(a1_2, a2_2);
}

template <>
void sorting_network_sort4<int>(int arr[]) {
    int a0 = arr[0];
    int a1 = arr[1];
    int a2 = arr[2];
    int a3 = arr[3];

    swap_if_greater(a0, a2);
    swap_if_greater(a1, a3);
    swap_if_greater(a0, a1);
    swap_if_greater(a2, a3);
    swap_if_greater(a1, a2);

    arr[0] = a0;
    arr[1] = a1;
    arr[2] = a2;
    arr[3] = a3;
}

int main(int argc, char** argv) {
    std::vector<int> array = create_random_array<int>(ARR_LEN, 0, ARR_LEN - 1);
    std::vector<int> copy(array);
    std::vector<int> result(array);

    {
        measure_time m("insertion_sort");
        for (int i = 0; i < ARR_LEN; i += 4) {
            insertion_sort<int, 4>(&array[i]);
        }
    }

    {
        measure_time m("quicksort");
        for (int i = 0; i < ARR_LEN; i += 4) {
            quicksort<int>(&copy[i], 0, 3);
        }
    }

    {
        measure_time m("sorting network sort");
        for (int i = 0; i < ARR_LEN; i += 4) {
            sorting_network_sort4<int>(&result[i]);
        }
    }

    if (result != array) {
        std::cout << "ERROR QUICKSORT\n";
    } else if (result != copy) {
        std::cout << "ERROR HEAPSORT\n";
    } else {
        std::cout << "SUCCESS\n";
    }

    std::random_shuffle(array.begin(), array.end());
    result = array;
    copy = array;

    {
        measure_time m("std::sort");
        std::sort(result.begin(), result.end());
    }

    {
        measure_time m("quicksort");
        quicksort(&array[0], 0, array.size() - 1);
    }

    {
        measure_time m("heapsort");
        heapsort(&copy[0], copy.size());
    }

    if (result != array) {
        std::cout << "ERROR QUICKSORT\n";
    } else if (result != copy) {
        std::cout << "ERROR HEAPSORT\n";
    } else {
        std::cout << "SUCCESS\n";
    }
}