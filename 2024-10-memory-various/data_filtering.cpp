#include "../common/argparse.h"
#include "likwid.h"

#include <limits>

template <typename T>
void fill_buffer(T* buff, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        buff[i] = static_cast<T>(rand());
    }
}

static void clobber() {
    asm volatile("" : : : "memory");
}

template <typename T>
void assert_buffers_equal(T const * const buff0, T const * const buff1, size_t const size) {
    for (size_t i = 0; i < size; ++i) {
        if (buff0[i] != buff1[i]) {
            std::cout << "Buffers not equal at position " << i << std::endl;
            return;
        }
    }
    std::cout << "Buffers equal\n";
}

struct filter_float {
    static constexpr uint32_t EOL = std::numeric_limits<uint32_t>::max();

    float val;
    uint32_t next;
};

template <typename T, typename T2>
T2 data_filtering(T* values, uint32_t start, float val, uint32_t& new_start) {
    uint32_t prev = T::EOL;
    uint32_t current = start;
    new_start = T::EOL;

    T2 max_value = std::numeric_limits<T2>::min();

    while(current != T::EOL) {
        if (values[current].val < val) {
            if (new_start == T::EOL) {
                new_start = current;
            }

            if (values[current].val > max_value) {
                max_value = values[current].val;
            }

            prev = current;
        } else {
            // Needs to be deleted since it doesn't fit the criterion
            if (prev != T::EOL) {
                values[prev].next = values[current].next;
            }
        }

        current = values[current].next;
    }

    return max_value;
}

template <typename T>
T data_filtering(T* values, size_t length, T val, size_t& new_length) {
    size_t j = 0;
    T max_value = std::numeric_limits<T>::min();
    for (size_t i = 0; i < length; i++) {
        if (values[i] < val) {
            values[j] = values[i];
            j++;
            if (values[i] > max_value) {
                max_value = values[i];
            }
        }
    }
    new_length = j;
    return max_value;
}

template <typename T1, typename T2>
void generate_data(size_t size, T1 * a, T2* b, size_t max_size) {
    for (size_t i = 0; i < size; i++) {
        T1 value = random() % max_size;
        a[i] = value;
        b[i].val = value;
        b[i].next = i+1;
    }
    b[size-1].next = T2::EOL;
}

template <typename T>
void print_array(T* a, size_t size) {
    for (size_t i = 0; i < size; i++) {
        std::cout << a[i] << ", ";
    }
    std::cout << std::endl;
}

template <typename T>
void print_filter_array(T* a, uint32_t start) {
    uint32_t current = start;
    while(current != T::EOL) {
        std::cout << a[current].val << ", ";
        current = a[current].next;
    }
    std::cout << std::endl;
}

template <typename T1, typename T2>
void compare_results(size_t size, T1* a, T2* b, uint32_t b_start) {
    uint32_t current = b_start;
    uint32_t current_count = 0;
    uint32_t i = 0;
    while (current != T2::EOL) {
        if (i >= size) {
            goto not_same;
        }

        if (a[i] != b[current].val) {
            goto not_same;
        }
        current_count++;
        i++;
        current = b[current].next;
    }

    if (current_count != size) {
        goto not_same;
    }
same:
    std::cout << "Arrays same ... " << std::endl;
    return;
not_same:
    std::cout << "Arrays not same... " << std::endl;
    std::cout << "Array: \n";
    print_array(a, size);
    print_filter_array(b, b_start);

    return;
}


using namespace argparse;

int main(int argc, const char* argv[]) {
    ArgumentParser parser("matrix_rotate", "matrix_rotate");

    parser.add_argument("-p", "--passes", "Number of filtering passes", true);
    parser.add_argument("-s", "--size", "Initial size of the filtering list", true);

    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return false;
    }
    
    size_t passes = 0;
    size_t size = 0;

    if (parser.exists("p")) {
        passes = parser.get<size_t>("p");
    }

    if (parser.exists("s")) {
        size = parser.get<size_t>("s");
    }

    LIKWID_MARKER_INIT;

    float * a1 = (float*) malloc(size * sizeof(float));
    filter_float *a2 = (filter_float*) malloc(size * sizeof(filter_float));

    generate_data(size, a1, a2, 10000);
    size_t filtered_size = 0;
    size_t filtered_start = 0;

    {
        LIKWID_MARKER_START("array");
        size_t new_size = size;
        float max_size = 10000;
        for (size_t p = 0; p < passes; p++) {
            max_size = data_filtering<float>(a1, new_size, max_size * 0.9f, new_size);
        }
        filtered_size = new_size;
        LIKWID_MARKER_STOP("array");
    }

    {
        LIKWID_MARKER_START("filter_list");
        uint32_t new_start = 0;
        float max_size = 10000;
        for (size_t p = 0; p < passes; p++) {
            max_size = data_filtering<filter_float, float>(a2, new_start, max_size * 0.9f, new_start);
        }
        filtered_start = new_start;
        LIKWID_MARKER_STOP("filter_list");
    }

    compare_results(filtered_size, a1, a2, filtered_start);

    std::cout << "end size " << filtered_size << "\n";

    LIKWID_MARKER_CLOSE;
}

