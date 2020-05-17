#include <chrono>
#include <iostream>
#include <random>     // mt19937 and uniform_int_distribution
#include <algorithm>  // generate
#include <vector>
#include <iterator>   // begin, end, and ostream_iterator
#include <functional> // bind



std::vector<int> createGrowingArray(int n, int stride, int inputArrayLen) {
    std::vector<int> v(n);
    int j = 0;
    for (int i = 0; i < n; i++) {
        v[i] = j;
        j = (j + stride) % inputArrayLen;
    }
    return v;
}

std::vector<int> createRandomArray(int n, int inputArrayLen) {
  std::random_device r;
  std::seed_seq      seed{r(), r(), r(), r(), r(), r(), r(), r()};
  std::mt19937       eng(seed); // a source of random data

  std::uniform_int_distribution<int> dist(0, inputArrayLen - 1);
  std::vector<int> v(n);

  generate(begin(v), end(v), bind(dist, eng));
  return v;
}


int generateRandomGrowingArray(int* array, int len) {
    constexpr int randomArrayLen = 6;
    int randomArray[randomArrayLen] = { 1, 2, 4, 3, 1, 2 };
    int j = 0;

    array[0] = 0;
    for (int i = 1; i < len; i++) {
        j = (j + 1) % randomArrayLen;
        array[i] = array[i - 1] + randomArray[j];
    }
    return array[len - 1];
}


template <bool withPrefetching>
int binarySearch(int* array, int number_of_elements, int key) {
    int low = 0, high = number_of_elements-1, mid;
    while(low <= high) {
        mid = (low + high)/2;
        if (withPrefetching) {
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
    std::vector<int> indexArray(1);
    int len = 4000000;
    int* inputArray;
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
        indexArray = createRandomArray(searches, len);
    } else {
        indexArray = createGrowingArray(searches, stride, len);
    }

    inputArray = (int*) malloc(sizeof(int) * len);
    generateRandomGrowingArray(inputArray, len);

    auto start_time = std::chrono::high_resolution_clock::now();
    if (prefetching) {
        for (int i = 0; i < searches; i++) {
            binarySearch<true>(inputArray, len, inputArray[indexArray[i]]);
        }
    }
    else {
        for (int i = 0; i < searches; i++) {
            binarySearch<false>(inputArray, len, inputArray[indexArray[i]]);
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto time = end_time - start_time;

    std::cout << " took " << time/std::chrono::milliseconds(1) << "ms to run.\n";

    free(inputArray);

    return 0;
}