#pragma once

#include <cstdint>
#include <iostream>

struct sorting_stats {
   uint64_t memory_accesses;
   uint64_t comparisons;
   uint64_t swaps;

   sorting_stats() :
    memory_accesses(0),
    comparisons(0),
    swaps(0)
   {}
};


std::ostream& operator<<(std::ostream& os, const sorting_stats& s)
{
    os << "Memory accesses = " << s.memory_accesses << ", comparisons = " << s.comparisons << ", swaps = " << s.swaps << std::endl;
    return os;
}


template <typename T>
void print_array(std::vector<T>& vec) {
    for(const auto& v: vec) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;
}

template <typename T>
void print_array2(std::vector<T>& vec, int low, int high) {
    for(int i = low; i <= high; i++) {
        std::cout << vec[i] << ", ";
    }
    std::cout << std::endl;
}

