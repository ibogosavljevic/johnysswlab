#pragma once

#include <cstddef>
#include <cstdio>
#include <stdlib.h>
#include <iostream>

namespace aligned {

template <typename T>
T* allocate_buffer(size_t size) {
    void* result;
    size_t buffer_size = size * sizeof(T) + 64;

#ifdef _WIN32
    result = _aligned_malloc(buffer_size, 64);
    return reinterpret_cast<T*>(result);
#else
    int errcode = posix_memalign(&result, 64, buffer_size);
    if (errcode) {
        return nullptr;
    } else {
        return reinterpret_cast<T*>(result);
    }
#endif
}

template <typename T>
void free_buffer(T* p) {
#ifdef _WIN32
    _aligned_free(p);
#else
    free(p);
#endif
}

}

template <typename T>
void fill_buffer(T* buff, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        buff[i] = static_cast<T>(rand());
    }
}

template <typename T, typename Func>
void fill_buffer(T* buff, size_t size, Func f) {
    for (size_t i = 0; i < size; ++i) {
        buff[i] = f();
    }
}

template <typename T>
void set_buffer(T* buff, size_t size, T val) {
    for (size_t i = 0; i < size; ++i) {
        buff[i] = val;
    }
}

template <typename T>
void copy_buffer(T* out, T* in, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        out[i] = in[i];
    }
}

#define equal(...) equal_internal(__VA_ARGS__, __FILE__, __LINE__)

template <typename T>
bool equal_internal(T buf1, T buf2, size_t size, const char* file, size_t line) {
    std::cout << "File " << file << ", line " << line << ": ";
    for (size_t i = 0; i < size; ++i) {
        if (!(buf1[i] == buf2[i])) {
            std::cout << "Buffers not same at index " << i << ": val1 = " << buf1[i] << ", val2 = " << buf2[i] << std::endl;
            return false;
        }
    }
    std::cout << "Buffers same " << std::endl;
    return true;
}

template <typename T>
bool equal_internal(T v1, T v2, const char* file, size_t line) {
    if (!(v1 == v2)) {
        std::cout << "File " << file << ", line " << line << ": ";
        std::cout << "Values not same " << v1<< " and " << v2 << std::endl;
        return false;
    } else {
        return true;
    }
}

template <typename T>
void equal_internal(const T* v, std::initializer_list<T> l, const char* file, size_t line) {
    size_t i = 0;
    for (auto it = l.begin(); it != l.end(); ++it, ++i) {
        if (!(v[i] == *it)) {
            std::cout << "File " << file << ", line " << line << ": ";
            std::cout << "At position " << i << ", expected " << *it << ", actual " << v[i] << "\n";
            return;
        }
    }
}

#ifdef near
#undef near
#endif

#define near(...) near_internal(__VA_ARGS__, __FILE__, __LINE__)

template <typename T>
bool near_internal(T v1, T v2, T max_diff, const char* file, size_t line) {
    T diff = std::abs(v1 - v2);
    std::cout << "File " << file << ", line " << line << ": ";
    if (diff > max_diff) {
        std::cout << "Values not near " << v1<< " and " << v2 
                  << ", diff " << diff << ", max_diff " << max_diff << std::endl;
        return false;
    } else {
        std::cout << "Values same " << std::endl;
        return true;
    }
}

template <typename T>
bool near_internal(T* buf1, T* buf2, T max_diff, size_t size, const char* file, size_t line) {
    std::cout << "File " << file << ", line " << line << ": ";
    for (size_t i = 0; i < size; ++i) {
        if (std::abs(buf1[i] - buf2[i]) > max_diff) {
            std::cout << "Buffers not same at position " << i << ": val1 = " << buf1[i] << ", val2 = " << buf2[i] << std::endl;
            return false;
        }
    }
    std::cout << "Buffers same " << std::endl;
    return true;
}

#ifdef _MSC_VER

#pragma optimize("", off)

static void clobber() {
    void *p = malloc(10);
    free(p);
}

#pragma optimize("", on)

#else

static void clobber() {
    asm volatile("" : : : "memory");
}
#endif

template <typename FUNC>
void run_test(int repeat_count, const std::string& name, FUNC f) {
    LIKWID_MARKER_START(name.c_str());
    for (int i = 0; i < repeat_count; ++i) {
        f();
        clobber();
    }
    
    LIKWID_MARKER_STOP(name.c_str());
}