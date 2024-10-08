#include "utils.h"

#include <chrono>
#include <iostream>
#include <vector>


template <bool with_prefetching>
int binary_search(int* array, int number_of_elements, int key) {
    int low = 0, high = number_of_elements-1, mid;
    while(low <= high) {
        mid = (low + high)/2;
        if (with_prefetching) {
            // low path
            __builtin_prefetch (&array[(mid + 1 + high)/2], 0, 1);
            // high path
            __builtin_prefetch (&array[(low + mid - 1)/2], 0, 1);
        }

        if(array[mid] < key)
            low = mid + 1; 
        else if(array[mid] == key)
             return mid;
        else if(array[mid] > key)
             high = mid-1;
        }
    return -1;
}

int main(int argc, char* argv[]) {
    bool prefetching = false;
    int stride = 0;
    std::vector<int> index_array(1);
    int len = 4000000;
    int* input_array;
    int searches = len;

    if (argc >= 2) {
        if(std::string(argv[1]).rfind("--working-set=") == 0) {
            len = std::atoi(argv[1] + 14);
        }
    }
    
    if (argc >= 3) {
        if (std::string(argv[2]) == "--prefetching-on") {
            prefetching = true;
        }
    }

    if (argc >= 4) {
        if (std::string(argv[3]).rfind("--stride=") == 0) {
            stride = std::atoi(argv[3] + 9);
        }
    }

    if (argc >= 5) {
        if (std::string(argv[4]).rfind("--searches=") == 0) {
            searches = std::atoi(argv[4] + 11);
        }
    }


    std::cout << "Binary search: \n";
    std::cout << "Working set = " << len << "\n";
    std::cout << "Software prefetching = " << (prefetching ? "on" : "off") << "\n";
    std::cout << "Keys distribution = " << (stride == 0 ? "random" : "stride = ");
    if (stride != 0) {
        std::cout << stride;
    }
    std::cout << "\n"; 
    std::cout << "Number of searches = " << searches << "\n";

    if (stride == 0) {
        index_array = create_random_array(searches, 0, len);
    } else {
        index_array = create_growing_array(searches, stride, len);
    }

    input_array = (int*) malloc(sizeof(int) * len);
    generate_random_growing_array(input_array, len);

    auto start_time = std::chrono::high_resolution_clock::now();
    if (prefetching) {
        for (int i = 0; i < searches; i++) {
            binary_search<true>(input_array, len, input_array[index_array[i]]);
        }
    }
    else {
        for (int i = 0; i < searches; i++) {
            binary_search<false>(input_array, len, input_array[index_array[i]]);
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto time = end_time - start_time;

    std::cout << " took " << time/std::chrono::milliseconds(1) << "ms to run.\n";

    free(input_array);

    return 0;
}