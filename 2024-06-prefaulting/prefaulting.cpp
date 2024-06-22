#include "likwid.h"
#include <random>
#include <iostream>
#include <sys/mman.h>

void fill_lookup_array(int32_t array[], size_t size) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6; // distribution in range [1, 6]

    for (size_t i = 0; i < size; i++) {
        int32_t val = dist6(rng);
        array[i] = val;
    }
}

int main() {
    LIKWID_MARKER_INIT;

    static constexpr size_t array_size = 4096*1024*128;
    static constexpr size_t array_memory_size = array_size * sizeof(int32_t);

    int32_t* array1 = (int32_t*) mmap(0, array_memory_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    int32_t* array2 = (int32_t*) mmap(0, array_memory_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, -1, 0);

    LIKWID_MARKER_START("Calloc");
    fill_lookup_array(array2, array_size);
    LIKWID_MARKER_STOP("Calloc");

    LIKWID_MARKER_START("Malloc");
    fill_lookup_array(array1, array_size);
    LIKWID_MARKER_STOP("Malloc");

    if (array1[array2[0] % array_size] == array2[array1[0] % array_size]) {
        std::cout << "Same\n";
    }

    munmap(array1, array_memory_size);
    munmap(array2, array_memory_size);

    LIKWID_MARKER_CLOSE;

    return 0;
}