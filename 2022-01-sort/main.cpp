#include <vector>
#include <iostream>

#include "heapsort.h"
#include "quicksort.h"
#include "utils.h"
#include <likwid.h>

static constexpr int ARR_SIZE = 32 * 1024 * 1024;
/*
template <typename T>
void print_array(std::vector<T>& vec) {
    for(const auto& v: vec) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;
}*/

int main(int argc, char** argv) {
    std::vector<float> quick_test_arr = create_random_array<float>(ARR_SIZE, 0, 100);
    std::vector<float> heap_test_arr(quick_test_arr);
    std::vector<float> heap_k_test_arr(quick_test_arr);

    bool do_quick = false;
    bool do_heap = false;
    bool do_heap4 = false;
    
    bool do_stats = false;

    if (argc > 1) {
        do_quick = std::string(argv[1]).find("q") != std::string::npos;
        do_heap = std::string(argv[1]).find("h") != std::string::npos;
        do_heap4 = std::string(argv[1]).find("k") != std::string::npos;
        do_stats = std::string(argv[1]).find("s") != std::string::npos;
    } else {
        std::cout << "Unknown args\n";
        return -1;
    }

    std::cout << "Quicksort = " << do_quick << ", heapsort = " << do_heap << std::endl;

    LIKWID_MARKER_INIT;

    if (do_quick) {
        if (do_stats) {
            LIKWID_MARKER_START("quicksort");
            sorting_stats stats = quicksort_stat(quick_test_arr);
            LIKWID_MARKER_STOP("quicksort");
            std::cout << "Quicksort stats: " << stats;
        } else {
            LIKWID_MARKER_START("quicksort");
            quicksort(quick_test_arr);
            LIKWID_MARKER_STOP("quicksort");
        }
    }

    if (do_heap) {
        if (do_stats) {
            LIKWID_MARKER_START("heapsort");
            sorting_stats stats = heapsort_stat(heap_test_arr);
            LIKWID_MARKER_STOP("heapsort");
            std::cout << "Heapsort stats: " << stats;
        } else {
            LIKWID_MARKER_START("heapsort");
            heapsort(heap_test_arr);
            LIKWID_MARKER_STOP("heapsort");
        }
    }

    if (do_heap4) {
        LIKWID_MARKER_START("heapsort_k");
        heapsort_k<float, 8>(heap_k_test_arr);
        LIKWID_MARKER_STOP("heapsort_k");

    }
    //print_array(quick_test_arr);
    //print_array(heap_test_arr);

    if (quick_test_arr != heap_test_arr) {
        std::cout << "QH: Not same\n";
    } else {
        std::cout << "QH: Same\n";
    }

    if (quick_test_arr != heap_k_test_arr) {
        std::cout << "QK: Not same\n";
    } else {
        std::cout << "QK: Same\n";
    }

    if (heap_test_arr != heap_k_test_arr) {
        std::cout << "HK: Not same\n";
    } else {
        std::cout << "HK: Same\n";
    }

    LIKWID_MARKER_CLOSE;

    return 0;
}