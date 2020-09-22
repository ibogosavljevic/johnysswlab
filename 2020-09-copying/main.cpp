#include <ostream>
#include "bit_field.h"
#include "measure_time.h"

using namespace jsl;

constexpr size_t arr_len = 10000000;
constexpr size_t arr_len_append = 10000;

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

    {
        measure_time m("regular");
        bit_field result = crunch_bitfield_regular(a, b);
        std::cout << "regular result " << result;
    }

    {
        measure_time m("compound");
        bit_field result = crunch_bitfield_compound(a, b);
        std::cout << "compound result " << result;
    }
    {
        measure_time m("append regular");
        bit_field result = append_regular(a, b);
        scratch |= result;
    }
    {
        measure_time m("append optimized");
        bit_field result = append_optimized(a, b, false);
        scratch |= result;
    }
    {
        measure_time m("append optimized with reserve");
        bit_field result = append_optimized(a, b, true);
        scratch |= result;
    }

    std::cout << scratch;

    return 0;
}
