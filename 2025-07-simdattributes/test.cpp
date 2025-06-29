#include <vector>
#include <random>
#include <iostream>

static void clobber() {
    asm volatile("" : : : "memory");
}


extern "C" {

__attribute__ ((__simd__ ("notinbranch"), const, nothrow)) 
double square(double x);

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

int main(int argc, char** argv) {
    static constexpr size_t s { 1000000000ULL };
    std::vector<double> input { generateRandomDoubles(s) };
    double res { 0.0 };
 
    double * d = input.data();
 
    for (size_t i = 0; i < s; i++) {
        d[i] = square(d[i]);
    }

    clobber();

    std::cout << "Res = " << res << "\n";
}
