#include <vector>
#include "utils.h"

#define LIKWID_PERFMON
#include <likwid.h>

float sum(std::vector<float>& arr, int repeat_count) {
    float result = 0.0;
    for (int k = 0; k < repeat_count; k++) {
        LIKWID_MARKER_START("Compute");
        for (int i = 0; i < arr.size(); i++) {
            result += arr[i];
        }
        LIKWID_MARKER_STOP("Compute");
    }
    return result;
}

int main(int argc, char** argv) {
    std::vector<float> test_array =
        create_random_array<float>(128 * 1024 * 1024, 0, 128 * 1024 * 1024);

    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;

    sum(test_array, 4);

    LIKWID_MARKER_CLOSE;
}