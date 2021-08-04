
#include "filter_vector.h"
#include <iostream>
#include <list>

#define LIKWID_PERFMON
#include <likwid.h>

template <typename T1, typename T2>
bool compare_list(T1& reference_list, T2& test_list) {
    auto ref_it = reference_list.begin();
    auto test_it = test_list.begin();
    for (; test_it != test_list.end();
         ++ref_it, test_it = test_list.next(test_it)) {
        if (*ref_it != test_list.at(test_it)) {
            std::cout << "ERROR at " << *ref_it << "\n";
            return false;
        }
    }

    if (ref_it != reference_list.end()) {
        std::cout << "ERROR, not end\n";
        return false;
    }

    return true;
}

int main(int argc, char** argv) {
    std::list<int> ref_filter;
    jsl::filter_vector<int> test_filter;

    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;

    static constexpr int list_size = 256 * 1024 * 1024;
    test_filter.reserve(list_size);

    LIKWID_MARKER_START("REF:INSERT");

    for (int i = 0; i < list_size; ++i) {
        ref_filter.push_back(i);
    }

    LIKWID_MARKER_STOP("REF:INSERT");

    LIKWID_MARKER_START("TEST:INSERT");

    for (int i = 0; i < list_size; ++i) {
        test_filter.push_back(i);
    }

    LIKWID_MARKER_STOP("TEST:INSERT");

    compare_list(ref_filter, test_filter);

    test_filter.compact();

    LIKWID_MARKER_START("REF:TRAVERSAL1");

    int sum = 0;
    for (auto it = ref_filter.begin(); it != ref_filter.end(); ++it) {
        sum += *it;
    }

    LIKWID_MARKER_STOP("REF:TRAVERSAL1");

    std::cout << "Sum ref = " << sum << std::endl;

    LIKWID_MARKER_START("TEST:TRAVERSAL1");

    sum = 0;
    for (auto it = test_filter.begin(); it != test_filter.end();
         it = test_filter.next(it)) {
        sum += test_filter.at(it);
    }

    LIKWID_MARKER_STOP("TEST:TRAVERSAL1");

    std::cout << "Sum test = " << sum << std::endl;


    compare_list(ref_filter, test_filter);

    LIKWID_MARKER_START("TEST:ERASURE");

    int count = 0;
    for (auto it = test_filter.begin(); it != test_filter.end();) {
        if (count != 0) {
            it = test_filter.erase(it);
            count--;
        } else {
            count = 5;
            it = test_filter.next(it);
        }
    }

    LIKWID_MARKER_STOP("TEST:ERASURE");

    LIKWID_MARKER_START("REF:ERASURE");
    count = 0;

    for (auto it = ref_filter.begin(); it != ref_filter.end();) {
        if (count != 0) {
            it = ref_filter.erase(it);
            count--;
        } else {
            count = 5;
            ++it;
        }
    }

    LIKWID_MARKER_STOP("REF:ERASURE");

    compare_list(ref_filter, test_filter);

    LIKWID_MARKER_START("REF:TRAVERSAL2");

    sum = 0;
    for (auto it = ref_filter.begin(); it != ref_filter.end(); ++it) {
        sum += *it;
    }

    LIKWID_MARKER_STOP("REF:TRAVERSAL2");

    std::cout << "Sum ref = " << sum << std::endl;

    LIKWID_MARKER_START("TEST:TRAVERSAL2");

    sum = 0;
    for (auto it = test_filter.begin(); it != test_filter.end();
         it = test_filter.next(it)) {
        sum += test_filter.at(it);
    }

    LIKWID_MARKER_STOP("TEST:TRAVERSAL2");

    std::cout << "Sum test = " << sum << std::endl;

    test_filter.compact();

    LIKWID_MARKER_START("TEST:COMPACT_TRAVERSAL2");

    sum = 0;
    for (auto it = test_filter.begin(); it != test_filter.end();
         it = test_filter.next(it)) {
        sum += test_filter.at(it);
    }

    LIKWID_MARKER_STOP("TEST:COMPACT_TRAVERSAL2");

    std::cout << "Sum test = " << sum << std::endl;

    compare_list(ref_filter, test_filter);

    LIKWID_MARKER_CLOSE;

    std::cout << "END\n";
}