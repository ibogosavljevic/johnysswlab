#include "likwid.h"
#include <iostream>
#include <string>
#include <vector>

using distance_t = float;

struct point_t {
    distance_t x;
    distance_t y;
};

std::ostream& operator<<(std::ostream& os, const point_t& dt) {
    os << "[ " << dt.x << ", " << dt.y << " ]";
    return os;
}


std::pair<size_t, size_t> distance_simple(point_t* p1, point_t* p2, distance_t pivot, distance_t* smaller, distance_t* larger, size_t n) {
    size_t smaller_size = 0;
    size_t larger_size = 0;

    LIKWID_MARKER_START(__FUNCTION__);
    for (size_t i = 0; i < n; i++) {
        distance_t x_diff = p1[i].x - p2[i].x;
        x_diff *= x_diff;
        distance_t y_diff = p1[i].y - p2[i].y;
        y_diff *= y_diff;
        distance_t distance = x_diff + y_diff;
        if (distance < pivot) { 
            smaller[smaller_size] = distance;
            smaller_size++;
        } else {
            larger[larger_size] = distance;
            larger_size++;
        }
    }
    LIKWID_MARKER_STOP(__FUNCTION__);
    return { smaller_size, larger_size };
}

std::pair<size_t, size_t> distance_fission(point_t* p1, point_t* p2, distance_t pivot, distance_t* smaller, distance_t* larger, size_t n) {
    size_t smaller_size = 0;
    size_t larger_size = 0;
    std::vector<distance_t> distance_vec(n);

    LIKWID_MARKER_START(__FUNCTION__);
    #pragma clang loop vectorize(enable)
    for (size_t i = 0; i < n; i++) {
        distance_t x_diff = p1[i].x - p2[i].x;
        x_diff *= x_diff;
        distance_t y_diff = p1[i].y - p2[i].y;
        y_diff *= y_diff;
        distance_vec[i] = x_diff + y_diff;
    }

    for (size_t i = 0; i < n; i++) {
        distance_t distance = distance_vec[i];
        if (distance < pivot) { 
            smaller[smaller_size] = distance;
            smaller_size++;
        } else {
            larger[larger_size] = distance;
            larger_size++;
        }
    }
    LIKWID_MARKER_STOP(__FUNCTION__);

    return { smaller_size, larger_size };
}

template <int SECTION_SIZE = 1024>
std::pair<size_t, size_t> distance_sectioning(point_t* p1, point_t* p2, distance_t pivot, distance_t* smaller, distance_t* larger, size_t n) {
    size_t smaller_size = 0;
    size_t larger_size = 0;
    std::vector<distance_t> distance_vec(SECTION_SIZE);

    LIKWID_MARKER_START(__FUNCTION__);
    for (size_t ii = 0; ii < n; ii += SECTION_SIZE) {
        int max_i = std::min(n, ii + SECTION_SIZE);
        #pragma clang loop vectorize(enable)
        for (size_t i = ii; i < max_i; i++) {
            distance_t x_diff = p1[i].x - p2[i].x;
            x_diff *= x_diff;
            distance_t y_diff = p1[i].y - p2[i].y;
            y_diff *= y_diff;
            distance_vec[i - ii] = x_diff + y_diff;
        }

        for (size_t i = ii; i < max_i; i++) {
            distance_t distance = distance_vec[i - ii];
            if (distance < pivot) { 
                smaller[smaller_size] = distance;
                smaller_size++;
            } else {
                larger[larger_size] = distance;
                larger_size++;
            }
        }
    }
    LIKWID_MARKER_STOP(__FUNCTION__);

    return { smaller_size, larger_size };
}

std::vector<point_t> generate_random_points(size_t size, int seed) {
    std::vector<point_t> result(size);
    for (size_t i = 0; i < size; i++) {
        int num1 = i % 4;
        int num2 = i % 7;
        int num3 = i % 11;
        int x = (num1 + num2 + seed - num3) % 9;
        int y = (-num1 - num2 + seed + num3) % 10;
        result[i].x = x + (1.0 / (i + 1));
        result[i].y = y - (1.0 / (i + 1));
    }

    return result;
}

template <typename T>
bool equal_until(const std::vector<T>& vec1, const std::vector<T>& vec2, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (vec1[i] != vec2[i]) {
            std::cout << "Diff at i = " << i << ", vec1 = " << vec1[i] << ", vec2 = " << vec2[i] <<  std::endl;
            return false;
        }
    }

    return true;
}

template <typename T>
void print_vec(const std::vector<T>& v) {
    for (int i = 0; i < v.size(); i++) {
        std::cout << v[i] << ", ";
    }
    std::cout << std::endl;
}

static constexpr int SIZE = 128*1024*1024;

int main(int argc, char **argv) {
    std::vector<distance_t> smaller_vec(SIZE);
    std::vector<distance_t> larger_vec(SIZE);

    std::vector<point_t> p1 = generate_random_points(SIZE, 4);
    std::vector<point_t> p2 = generate_random_points(SIZE, 9);

    static const distance_t PIVOT = 50.0;

    LIKWID_MARKER_INIT;

    std::pair<size_t, size_t> res = distance_simple(p1.data(), p2.data(), PIVOT, smaller_vec.data(), larger_vec.data(), SIZE);
    std::cout << "1) small size = " << res.first << ", large size = " << res.second << "\n";

    {
        std::vector<distance_t> smaller_vec2(SIZE);
        std::vector<distance_t> larger_vec2(SIZE);

        std::pair<size_t, size_t> res = distance_fission(p1.data(), p2.data(), PIVOT, smaller_vec2.data(), larger_vec2.data(), SIZE);

        std::cout << "2) small size = " << res.first << ", large size = " << res.second << "\n";
        if (!equal_until(smaller_vec, smaller_vec2, res.first)) {
            std::cout << "NOT SAME1\n";
        }

        if (!equal_until(larger_vec, larger_vec2, res.second)) {
            std::cout << "NOT SAME2\n";
        } 
    }

    {
        std::vector<distance_t> smaller_vec2(SIZE);
        std::vector<distance_t> larger_vec2(SIZE);

        std::pair<size_t, size_t> res = distance_sectioning(p1.data(), p2.data(), PIVOT, smaller_vec2.data(), larger_vec2.data(), SIZE);

        std::cout << "3) small size = " << res.first << ", large size = " << res.second << "\n";
        if (!equal_until(smaller_vec, smaller_vec2, res.first)) {
            std::cout << "NOT SAME3\n";
        }

        if (!equal_until(larger_vec, larger_vec2, res.second)) {
            std::cout << "NOT SAME4\n";
        } 
    }

    LIKWID_MARKER_CLOSE;

    return 0;
}