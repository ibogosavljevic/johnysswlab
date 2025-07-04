#include <vector>
#include <random>
#include <iostream>
#include <cassert>

static void clobber() {
    asm volatile("" : : : "memory");
}


#pragma omp declare simd
__attribute__ ((const, nothrow)) 
double square(double x);


std::vector<double> generateRandomDoubles(std::size_t size) {
    std::vector<double> result;
    result.reserve(size);  // Reserve memory for efficiency

    std::random_device rd;  // Seed generator
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_real_distribution<> dist(-100.0, 100.0);

    for (std::size_t i = 0; i < size; ++i) {
        result.push_back(dist(gen));
    }

    return result;
}

int main(int argc, char** argv) {
    static constexpr size_t s { 100000ULL };
    std::vector<double> input { generateRandomDoubles(s) };
    std::vector<double> res0(input.size());
    std::vector<double> res1(input.size());
 
    double * d = input.data(); 
    double * r0 = res0.data();
    double * r1 = res1.data();
 
    for (size_t i = 0; i < s; i++) {
        r0[i] = square(d[i]);
    }

    clobber();

    for (size_t i = 0; i < s; i++) {
        if (d[i] > 0) {
            r1[i] = square(d[i]);
        }
    }

    clobber();

    for (size_t i = 0; i < s; i++) {
        if (d[i] > 0) {
            assert(res0[i] == res1[i]);
        } else {
            assert(res1[i] == 0.0);
        }

    }

}
