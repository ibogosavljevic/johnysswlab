#include "vector_list.h"
#include <forward_list>
#include <iostream>
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
    std::forward_list<int> reference_list;
    jsl::vector_list<int> test_list;

    static constexpr int NUM = 128 * 1024 * 1024;

    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;

    LIKWID_MARKER_START("REF:PUSH_FRONT");

    for (int i = 0; i < NUM; i++) {
        reference_list.push_front(i);
    }

    LIKWID_MARKER_STOP("REF:PUSH_FRONT");

    LIKWID_MARKER_START("TEST:PUSH_FRONT");

    for (int i = 0; i < NUM; i++) {
        test_list.push_front(i);
    }

    LIKWID_MARKER_STOP("TEST:PUSH_FRONT");

    compare_list(reference_list, test_list);

    int count = 0;

    LIKWID_MARKER_START("TEST:ERASE");

    for (auto test_it = test_list.begin(); test_it != test_list.end();
         test_it = test_list.next(test_it)) {
        if (count == 0) {
            test_it = test_list.erase_after(test_it);
            count = 4;
        }
    }

    LIKWID_MARKER_STOP("TEST:ERASE");

    count = 0;

    LIKWID_MARKER_START("REF:ERASE");

    for (auto ref_it = reference_list.begin(); ref_it != reference_list.end();
         ++ref_it) {
        if (count == 0) {
            ref_it = reference_list.erase_after(ref_it);
            count = 4;
        }
    }
    LIKWID_MARKER_STOP("REF:ERASE");

    compare_list(reference_list, test_list);

    for (int i = 0; i < 4; i++) {
        reference_list.pop_front();
        test_list.pop_front();
    }

    compare_list(reference_list, test_list);

    count = 0;

    LIKWID_MARKER_START("TEST:INSERT_AFTER");

    for (auto test_it = test_list.begin(); test_it != test_list.end();
         test_it = test_list.next(test_it)) {
        if (count == 0) {
            test_it = test_list.insert_after(test_it, 4);
            count = 4;
        }
    }

    LIKWID_MARKER_STOP("TEST:INSERT_AFTER");

    count = 0;

    LIKWID_MARKER_START("REF:INSERT_AFTER");

    for (auto ref_it = reference_list.begin(); ref_it != reference_list.end();
         ++ref_it) {
        if (count == 0) {
            ref_it = reference_list.insert_after(ref_it, 4);
            count = 4;
        }
    }
    LIKWID_MARKER_STOP("REF:INSERT_AFTER");

    LIKWID_MARKER_START("TEST:SUM_BEFORE_COMPACT");

    int sum = 0;
    for (auto test_it = test_list.begin(); test_it != test_list.end();
         test_it = test_list.next(test_it)) {
        sum += test_list.at(test_it);
    }

    LIKWID_MARKER_STOP("TEST:SUM_BEFORE_COMPACT");

    std::cout << "SUM = " << sum << std::endl;

    test_list.compact();

    LIKWID_MARKER_START("TEST:SUM_AFTER_COMPACT");

    sum = 0;
    for (auto test_it = test_list.begin(); test_it != test_list.end();
         test_it = test_list.next(test_it)) {
        sum += test_list.at(test_it);
    }

    LIKWID_MARKER_STOP("TEST:SUM_AFTER_COMPACT");

    std::cout << "SUM = " << sum << std::endl;

    compare_list(reference_list, test_list);

    std::cout << "END\n";

    LIKWID_MARKER_CLOSE;
}