#pragma once

#include <cstddef>
#include <vector>
#include <iostream>

#include "complex.h"


struct complex_soa {
    std::vector<float> re;
    std::vector<double> im;

    complex_soa(size_t size) {
        re.resize(size);
        im.resize(size);
    }

    struct complex_proxy {
        float* re;
        double* im;

        complex_proxy(float* rep, double* imp) : re(rep), im(imp) {} 
        complex_proxy() {}

        bool operator<(const complex_proxy& other) const {
            return (*re + *im) < (*other.re + *other.im);
        }

        bool operator<=(const complex_proxy& other) const {
            return (*re + *im) <= (*other.re + *other.im);
        }

        bool operator<(const complex_simple& other) const {
            return (*re + *im) < (other.re + other.im);
        }

        template <typename Complex>
        bool operator==(const Complex& c) const {
            return *re == c.re && *im == c.im;
        }

        void swap(complex_proxy&& other) {
            std::swap(*re, *other.re);
            std::swap(*im, *other.im);
        }
    };

    complex_proxy operator[] (size_t index) {
        return complex_proxy(re.data() + index, im.data() + index);
    }

    void print() {
        std::cout << "{ ";
        for (int i = 0; i < re.size(); ++i) {
            std::cout << "[ " << re[i] << ", " << im[i] << " ], ";
        }
        std::cout << "}\n";
    }

    template <typename Complex>
    bool equal(const std::vector<Complex>& v) {
        if (v.size() != re.size()) {
            return false;
        }

        for (int i = 0; i < v.size(); ++i) {
            if (re[i] != v[i].re || im[i] != v[i].im) {
                return false;
            }
        }

        return true;
    }

};

size_t lower_bound(complex_soa& arr, size_t N, const complex_soa::complex_proxy& X)
{
    size_t mid;
 
    // Initialise starting index and
    // ending index
    size_t low = 0;
    size_t high = N;
 
    // Till low is less than high
    while (low < high) {
        mid = low + (high - low) / 2;
 
        // If X is less than or equal
        // to arr[mid], then find in
        // left subarray
        if (X <= arr[mid]) {
            high = mid;
        }
 
        // If X is greater arr[mid]
        // then find in right subarray
        else {
            low = mid + 1;
        }
    }
   
    // if X is greater than arr[n-1]
    if(low < N && arr[low] < X) {
       low++;
    }
       
    // Return the lower_bound index
    return low;
}

int partition(complex_soa& arr, int low, int high)
{
    // Choosing the pivot
    complex_simple pivot (*arr[high].re, *arr[high].im);
 
    // Index of smaller element and indicates
    // the right position of pivot found so far
    int i = (low - 1);
 
    for (int j = low; j <= high - 1; j++) {
 
        // If current element is smaller than the pivot
        if (arr[j] < pivot) {
 
            // Increment index of smaller element
            i++;
            arr[i].swap(arr[j]);
        }
    }
    arr[i + 1].swap(arr[high]);
    return (i + 1);
}

void quicksort(complex_soa& arr, int low, int high)
{
    if (low < high) {
 
        // pi is partitioning index, arr[p]
        // is now at right place
        int pi = partition(arr, low, high);
 
        // Separately sort elements before
        // partition and after partition
        quicksort(arr, low, pi - 1);
        quicksort(arr, pi + 1, high);
    }
}

void quicksort(complex_soa& arr) {
    return quicksort(arr, 0, arr.re.size() - 1);
}

void multiply(complex_soa& result, complex_soa& a, complex_soa& b, int n) {
    for (int i = 0; i < n; ++i) {
        result.re[i] = a.re[i] * b.re[i] - a.im[i] * b.im[i];
        result.im[i] = a.re[i] * b.im[i] + a.im[i] + b.re[i];
    }
}

complex_soa generate_random(size_t size) {
    complex_soa result(size);
    for (int i = 0; i < size; ++i) {
        result.re[i] = rand();
        result.im[i] = rand();
    }
    return result;
}

