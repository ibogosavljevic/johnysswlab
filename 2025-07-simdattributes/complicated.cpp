    #include <cstdint>
    #include <vector>
    #include <cmath>
    #include <random>
    #include <cassert>
    #include <iostream>

    #pragma omp declare simd uniform(c) linear(i) simdlen(4) inbranch
    __attribute__ ((const, nothrow)) 
    double my_func(double x, int32_t k, double c, int32_t i) ;

    double my_func_scalar(double x, int32_t k, double c, int32_t i) {
        return x*c + i + k;
    }


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


int main() {

    static constexpr size_t s = 32;

    std::vector<double> in { generateRandomDoubles(s) };
    std::vector<double> res0 (s);
    std::vector<double> res1 (s);

    double const c = 0.721;

    for (size_t i = 0; i < s; i++) {
        std::cout << in[i] << ", ";
        if (in[i] > 0) {
            res0[i] = my_func_scalar(in[i], i%4, c, i);
        }
    }

    std::cout << "\n\n\n";

    #pragma omp simd
    for (size_t i = 0; i < s; i++) {
        if (in[i] > 0) {
            res1[i] = my_func(in[i], i%4, c, i);
        }
    }

    for (size_t i = 0; i < s; i++) {
        assert(res0[i] == res1[i]);
    }


}