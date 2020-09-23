#include <cassert>
#include <ostream>
#include <vector>
#include "bit_field.h"
#include "measure_time.h"

using namespace jsl;

constexpr size_t arr_len = 10000000;
constexpr size_t arr_len_append = 4;

std::ostream& operator<<(std::ostream& os, const jsl::bit_field& b) {
    b.stream_to(os);
    return os;
}

bit_field crunch_bitfield_regular(bit_field a, bit_field b) {
    bit_field result(a);
    for (int i = 0; i < arr_len; i++) {
        result = result & a;
        result = result | b;
    }
    return result;
}

bit_field crunch_bitfield_compound(bit_field a, bit_field b) {
    bit_field result(a);
    for (int i = 0; i < arr_len; i++) {
        result &= a;
        result |= b;
    }
    return result;
}

bit_field append_regular(bit_field a, bit_field b) {
    bit_field result({0});

    for (int i = 0; i < arr_len_append; i++) {
        result = result.append_t(a);
        result = result.append_t(b);
    }

    return result;
}

bit_field append_optimized(bit_field a, bit_field b, bool reserve) {
    bit_field result({0});

    if (reserve) {
        result.reserve((a.get_size() + b.get_size()) * arr_len_append);
    }

    for (int i = 0; i < arr_len_append; i++) {
        result.append(a);
        result.append(b);
    }

    return result;
}

int main(int argc, char* argv[]) {
    bit_field a({2, 2, 2, 2});
    bit_field b({1, 1, 1, 1});
    bit_field scratch({0, 0, 0, 0});
    std::vector<bit_field> result;
    result.reserve(10);

    {
        measure_time m("regular");
        result.emplace_back(crunch_bitfield_regular(a, b));
        std::cout << "regular result " << result[0];
    }
    {
        measure_time m("compound");
        result.emplace_back(crunch_bitfield_compound(a, b));
        std::cout << "compound result " << result[1];
    }
    assert(result[0] == result[1]);
    {
        measure_time m("append regular");
        result.emplace_back(append_regular(a, b));
        scratch |= result[2];
    }
    {
        measure_time m("append optimized");
        result.emplace_back(append_optimized(a, b, false));
        scratch |= result[3];
    }
    {
        measure_time m("append optimized with reserve");
        result.emplace_back(append_optimized(a, b, true));
        scratch |= result[4];
    }
    std::cout << result[2] << std::endl;
    std::cout << result[3] << std::endl;
    assert(result[3] == result[4]);
    assert(result[2] == result[3]);

    std::cout << scratch;

    return 0;
}
