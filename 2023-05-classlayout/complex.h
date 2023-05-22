#pragma once

#include <cstddef>
#include <vector>

template <int padding1, int padding2>
struct complex_t {
    float re;
    int p1[padding1];
    double im;
    int p2[padding2];

    complex_t() {}
    complex_t(float ret, double imt) : re(ret), im(imt) {}

    bool operator<(const complex_t& other) const {
        return (re + im) < (other.re + other.im);
    }

    bool operator<=(const complex_t& other) const {
        return (re + im) <= (other.re + other.im);
    }

    bool operator==(const complex_t& other) const {
        return re == other.re && im == other.im;
    }
};

using complex_simple = complex_t<0, 0>;

struct __attribute__((__packed__)) complex_packed  {
    float re;
    double im;

    bool operator<(const complex_packed& other) const {
        return (re + im) < (other.re + other.im);
    }

    bool operator<=(const complex_packed& other) const {
        return (re + im) <= (other.re + other.im);
    }
};

template <typename Complex>
void multiply(Complex* result, Complex* a, Complex* b, int n) {
    for (int i = 0; i < n; ++i) {
        result[i].re = a[i].re * b[i].re - a[i].im * b[i].im;
        result[i].im = a[i].re * b[i].im + a[i].im + b[i].re;
    }
}

template <typename Complex>
size_t lower_bound(Complex* arr, size_t N, const Complex& X)
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

template <typename Complex>
std::vector<Complex> generate_random(size_t size) {
    std::vector<Complex> result;
    result.resize(size);
    for (int i = 0; i < size; ++i) {
        result[i].re = rand();
        result[i].im = rand();
    }
    return result;
}