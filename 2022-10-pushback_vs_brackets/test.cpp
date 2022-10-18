#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <memory>
#include <cstring>
#include "likwid.h"

template <typename T>
struct my_vector {
    my_vector(size_t s) : val(new T[s]), size(s) {}
    ~my_vector() { if (size > 0) { delete[] val; } }

    my_vector (const my_vector&) = delete;
    my_vector (my_vector&& v): val(v.val), size(v.size) {
        v.size = 0;
        v.val = nullptr;
    }
    my_vector& operator= (const my_vector&) = delete;

    bool operator!= (const std::vector<T>& v) const {
        if (v.size() == size) { 
            for (int i = 0; i < size; i++) {
                if (val[i] != v[i]) { return true; }
            }
            return false;
        } else {
            return true;
        }
    }

    T* val;
    size_t size;
};


template <class T>
class malloc_wrapper
{
    public:

        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;
        typedef T *pointer;
        typedef const T *const_pointer;
        typedef T &reference;
        typedef const T &const_reference;
        typedef T value_type;

        template <class U>
        struct rebind
        {
            typedef malloc_wrapper<U> other;
        };

        pointer allocate(size_type n, const void *hint = 0)
        {
            std::cout << "Allocate, size = " << n << " bytes" << std::endl;
            pointer p = (pointer) malloc(n * sizeof(T));
            memset(p, 0, n * sizeof(T));
            return p;
        }

        void deallocate(pointer p, size_type n)
        {
            free(p);
        }

        void construct(pointer p, const_reference val)
        {
            new (p) T(val);
        }

        void destroy(pointer p)
        {
            p->~T();
        }
};

std::vector<double> calculate_sqrt_emplaceback(const std::vector<double>& in) {
    std::vector<double> result;
    result.reserve(in.size());
    size_t size = in.size();

    LIKWID_MARKER_START("EMPLACEBACK");
    for (size_t i = 0; i < size; i++) {
        result.emplace_back(std::sqrt(in[i]));
    }
    LIKWID_MARKER_STOP("EMPLACEBACK");

    return result;
}

std::vector<double> calculate_sqrt_brackets(const std::vector<double>& in) {
    std::vector<double> result(in.size());
    size_t size = in.size();

    LIKWID_MARKER_START("OPERATOR_BRACKETS");
    for (size_t i = 0; i < size; i++) {
        result[i] = std::sqrt(in[i]);
    }
    LIKWID_MARKER_STOP("OPERATOR_BRACKETS");

    return result;
}


std::vector<double> calculate_sqrt_brackets_scalar(const std::vector<double>& in) {
    std::vector<double> result(in.size());
    size_t size = in.size();

    LIKWID_MARKER_START("OPERATOR_SCALAR_BRACKETS");
    #pragma clang loop vectorize(disable)
    for (size_t i = 0; i < size; i++) {
        result[i] = std::sqrt(in[i]);
    }
    LIKWID_MARKER_STOP("OPERATOR_SCALAR_BRACKETS");

    return result;
}


my_vector<double> calculate_sqrt_my_vector(const std::vector<double>& in) {
    my_vector<double> result(in.size());
    size_t size = in.size();
    double* val = result.val;

    LIKWID_MARKER_START("CARRAY_OPERATOR_BRACKETS");
    for (size_t i = 0; i < size; i++) {
        val[i] = std::sqrt(in[i]);
    }
    LIKWID_MARKER_STOP("CARRAY_OPERATOR_BRACKETS");

    return result;
}


std::vector<double, malloc_wrapper<double>> calculate_sqrt_emplaceback_preinitialized(const std::vector<double>& in) {
    std::vector<double, malloc_wrapper<double>> result(in.size());
    result.reserve(in.size());
    size_t size = in.size();

    LIKWID_MARKER_START("OPERATOR_EMPLACEBACK_PREINITIALIZED");
    for (size_t i = 0; i < size; i++) {
        result.emplace_back(std::sqrt(in[i]));
    }
    LIKWID_MARKER_STOP("OPERATOR_EMPLACEBACK_PREINITIALIZED");

    return result;
}

bool are_same(const std::vector<double>& v1, const std::vector<double, malloc_wrapper<double>>& v2) {
    if (v1.size() != v2.size()) {
        return false;
    }

    for (int i = 0; i < v1.size(); i++) {
        if (v1[i] != v2[i]) { return false; }
    }

    return true;
}

int main(int argc, char **argv) {
    constexpr size_t SIZE = 256*1024*1024;
    std::vector<double> input(SIZE);

    LIKWID_MARKER_INIT;
    
    std::iota(input.begin(), input.end(), 0.0);
    std::random_shuffle(input.begin(), input.end());

    LIKWID_MARKER_START("EMPLACEBACK_TOTAL");
    std::vector<double> res1 = calculate_sqrt_emplaceback(input);
    LIKWID_MARKER_STOP("EMPLACEBACK_TOTAL");

    {
        LIKWID_MARKER_START("OPERATOR_BRACKETS_TOTAL");
        std::vector<double> res2 = calculate_sqrt_brackets(input);
        LIKWID_MARKER_STOP("OPERATOR_BRACKETS_TOTAL");
        if (res1 != res2) {
            std::cout << "Different\n";
        }
    }

    {
        LIKWID_MARKER_START("CARRAY_OPERATOR_BRACKETS_TOTAL");
        my_vector<double> res3 = calculate_sqrt_my_vector(input);
        LIKWID_MARKER_STOP("CARRAY_OPERATOR_BRACKETS_TOTAL");
        if (res3 != res1) {
            std::cout << "Different2\n";
        }
    }

    {
        LIKWID_MARKER_START("OPERATOR_EMPLACEBACK_PREINITIALIZED_TOTAL");
        std::vector<double, malloc_wrapper<double>> res4 = calculate_sqrt_emplaceback_preinitialized(input);
        LIKWID_MARKER_STOP("OPERATOR_EMPLACEBACK_PREINITIALIZED_TOTAL");
        if (!are_same(res1, res4)) {
            std::cout << "Different3\n";
        }
    }

    {
        LIKWID_MARKER_START("OPERATOR_BRACKETS_SCALAR_TOTAL");
        std::vector<double> res2 = calculate_sqrt_brackets_scalar(input);
        LIKWID_MARKER_STOP("OPERATOR_BRACKETS_SCALAR_TOTAL");
        if (res1 != res2) {
            std::cout << "Different\n";
        }
    }

    std::cout << "End\n";

    LIKWID_MARKER_CLOSE;

    return 0;
}