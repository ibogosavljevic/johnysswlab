#include <cstdlib>
#include <utility>
#include <iostream>
#include "likwid.h"

struct my_string {
    char* ptr;
    size_t size;

    my_string(size_t s) : ptr((char*)malloc(s)), size(s) { }
    my_string(const char* c) {
        size = strlen(c);
        ptr = (char*) malloc(size);
        memcpy(ptr, c, size);
    }
    ~my_string() { free(ptr); }

    void __attribute__((noinline)) replace(char find, char replaceWith) {
        for (int i = 0; i < size; i++) {
            if (ptr[i] == find) {
                ptr[i] = replaceWith;
            }
        }
    }

    void __attribute__((noinline)) replace2(char find, char replaceWith) {
        size_t size_priv = size;
        char* ptr_priv = ptr;
        for (int i = 0; i < size_priv; i++) {
            if (ptr_priv[i] == find) {
                ptr_priv[i] = replaceWith;
            }
        }
    }

    std::pair<bool, size_t> __attribute__((noinline)) find_substring(const my_string& substr) {
        if (substr.size > size) {
            return { false, 0 };
        }

        size_t s = size - substr.size;
        char* ptr_str = ptr;
        char* ptr_substr = substr.ptr;
        size_t size_substr = substr.size;
        for (int i = 0; i < s; ++i) {
            bool found = true;
            for (int j = 0; j < size_substr; j++) {
                if (ptr_str[i + j] != ptr_substr[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return { true, i };
            }
        }

        return { false, 0 };
    }


    std::pair<bool, size_t> __attribute__((noinline)) find_substring2(const my_string& substr) {
        if (substr.size > size) {
            return { false, 0 };
        }

        static constexpr size_t substring_buffer_max_size = 8;
        char substring_buffer[substring_buffer_max_size];
        size_t substring_buffer_size = std::min(substring_buffer_max_size, substr.size);
        for (int i = 0; i < substring_buffer_size; ++i) {
            substring_buffer[i] = substr.ptr[i];
        }

        size_t s = size - substr.size;
        char* ptr_str = ptr;
        char* ptr_substr = substr.ptr;
        size_t size_substr = substr.size;
        for (int i = 0; i < s; ++i) {
            bool found = true;
            int j;
            for (j = 0; j < substring_buffer_size; ++j) {
                if (ptr_str[i + j] != substring_buffer[j]) {
                    found = false;
                    break;
                }
            }

            if (found) {
                for (; j < size_substr; ++j) {
                    if (ptr_str[i + j] != ptr_substr[j]) {
                        found = false;
                        break;
                    }
                }
            }

            if (found) {
                return { true, i };
            }
        }

        return { false, 0 };
    }
};


template <typename T>
T generate(size_t s) {
    T res(s);
    for (int i = 0; i < s; i++) {
        res.ptr[i] = i;
    }
    return res;
}

int main(int argc, char** argv) {
    LIKWID_MARKER_INIT;

    static constexpr int size = 256*1024*1024;

    my_string str_simple = generate<my_string>(size);
    my_string str_restrict = generate<my_string>(size);

    LIKWID_MARKER_START("aliasing_simple");
    str_simple.replace('.', '?');
    LIKWID_MARKER_STOP("aliasing_simple");

    LIKWID_MARKER_START("aliasing_avoid");
    str_restrict.replace2('.', '?');
    LIKWID_MARKER_STOP("aliasing_avoid");

    LIKWID_MARKER_START("find_simple");
    auto res1 = str_simple.find_substring(my_string("IvicaBogosavljevic"));
    LIKWID_MARKER_STOP("find_simple");

    LIKWID_MARKER_START("find_subarray_caching");
    auto res2 = str_simple.find_substring2(my_string("IvicaBogosavljevic"));
    LIKWID_MARKER_STOP("find_subarray_caching");

    if (res1 != res2) {
        std::cout << "Not same\n";
    }
    std::cout << "Found = " << res1.first << "\n";

    LIKWID_MARKER_CLOSE;
}