#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#define LIKWID_PERFMON
#include <likwid.h>

template <typename T>
struct node {
    T value;
    node* next;
};

enum list_type_e { SPARSE, TIGHT, PERFECT };

node<double>* create_double_list(list_type_e type,
                                 int size,
                                 std::vector<node<double>>& backing_vector) {
    std::vector<int> indexes;

    if (type == SPARSE) {
        indexes.resize(size * 4);
        backing_vector.resize(indexes.size());
    } else {
        indexes.resize(size);
        backing_vector.resize(size);
    }

    std::iota(indexes.begin(), indexes.end(), 0.0);

    if (type != PERFECT) {
        std::random_shuffle(indexes.begin(), indexes.end());
    }

    backing_vector[indexes[0]].value = 0.0;
    for (int i = 1; i < size; i++) {
        backing_vector[indexes[i]].value = static_cast<double>(i);
        backing_vector[indexes[i - 1]].next = &backing_vector[indexes[i]];
    }

    backing_vector[indexes[size - 1]].next = nullptr;

    return &backing_vector[indexes[0]];
}

double sum(node<double>* start) {
    double r = 0.0;
    auto n = start;

    while (n != nullptr) {
        r += n->value;
        n = n->next;
    }

    return r;
}

int main(int argc, char** argv) {
    static constexpr int MAX_SIZE = 64 * 1024 * 1024;

    std::vector<list_type_e> list_types = {SPARSE, TIGHT, PERFECT};
    std::vector<int> size = {128, 1024 * 1024, MAX_SIZE};

    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;

    for (auto s : size) {
        for (auto l : list_types) {
            std::vector<node<double>> backing_vector;

            auto list_start = create_double_list(l, s, backing_vector);
            std::string name =
                "TYPE_" + std::to_string(l) + "_" + "SIZE_" + std::to_string(s);

            std::cout << name << std::endl;

            int repeat_count = MAX_SIZE / s;

            double result = 0.0;

            LIKWID_MARKER_START(name.c_str());
            for (int i = 0; i < repeat_count; i++) {

                result += sum(list_start);

            }
            LIKWID_MARKER_STOP(name.c_str());

            std::cout << "Result = " << result << std::endl;
        }
    }

    LIKWID_MARKER_CLOSE;
}