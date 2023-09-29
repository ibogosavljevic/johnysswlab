
#include <vector>
#include <variant>
#include <random>
#include <algorithm>
#include <iostream>
#include "likwid.h"

struct base_class {
    int val;
    virtual int get_value() = 0;
    virtual ~base_class() = default;
};

struct derived1 : public base_class {
    int get_value() override {
        return val + 1;
    }
};

struct derived2 : public base_class {
    int get_value() override {
        return val + 2;
    }
};

struct derived3 : public base_class {
    int get_value() override {
        return val + 3;
    }
};

struct test_vectors {
    std::vector<std::variant<derived1, derived2, derived3>> vec_variant;
    std::vector<base_class*> vec_pointer;
    std::vector<base_class*> vec_pointer_shuffled;

    ~test_vectors() {
        for (base_class* c: vec_pointer) {
            delete c;
        }
    }
};

test_vectors create_test_vector(size_t size) {
    test_vectors t;
    for (size_t i = 0; i < size; i++) {
        int y = (i % 21) + (i % 25) + (i % 44);
        int x = y % 3;
        switch (x) {
            case 0: {
                derived1 d;
                d.val = y;
                t.vec_variant.push_back(d);
                t.vec_pointer.push_back(new derived1(d));
                break;
            }
            case 1: {
                derived2 d;
                d.val = y;
                t.vec_variant.push_back(d);
                t.vec_pointer.push_back(new derived2(d));
                break;
            }
            case 2: {
                derived3 d;
                d.val = y;
                t.vec_variant.push_back(d);
                t.vec_pointer.push_back(new derived3(d));
                break;
            }
        }
    }

    t.vec_pointer_shuffled.resize(size);

    std::copy(t.vec_pointer.begin(), t.vec_pointer.end(), t.vec_pointer_shuffled.begin());

    std::random_device rng;
    std::mt19937 urng(rng());
    std::shuffle(t.vec_pointer_shuffled.begin(), t.vec_pointer_shuffled.end(), urng);

    return t;
}

int main(int argc, const char* argv[]) {
    static constexpr size_t size = 128*1024*1024;
    test_vectors t = create_test_vector(size);

    LIKWID_MARKER_INIT;

    int found = 0;
    LIKWID_MARKER_START("std_variant");
    for (size_t i = 0; i < size; i++) {
        found += std::visit([&](auto& v) -> int { return v.get_value(); }, t.vec_variant[i]) < 5;
    }
    LIKWID_MARKER_START("std_variant");
    std::cout << "Found = " << found << "\n";

    found = 0;
    LIKWID_MARKER_START("array_pointer_low_fragmentation");
    for (size_t i = 0; i < size; i++) {
        found += t.vec_pointer[i]->get_value() < 5;
    }
    LIKWID_MARKER_START("array_pointer_low_fragmentation");
    std::cout << "Found = " << found << "\n";

    found = 0;
    LIKWID_MARKER_START("array_pointer_high_fragmentation");
    for (size_t i = 0; i < size; i++) {
        found += t.vec_pointer_shuffled[i]->get_value() < 5;
    }
    LIKWID_MARKER_START("array_pointer_high_fragmentation");
    std::cout << "Found = " << found << "\n";

    LIKWID_MARKER_CLOSE;

    return 0;
}