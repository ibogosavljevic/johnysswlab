#include <cassert>
#include <ostream>
#include <vector>
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
        result.reserve((a.get_size() + b.get_size()) * arr_len_append +
                       result.get_size());
    }

    for (int i = 0; i < arr_len_append; i++) {
        result.append(a);
        result.append(b);
    }

    return result;
}

std::vector<bit_field> create_bitfield_vector(size_t count) {
    std::vector<bit_field> result;
    result.reserve(count);

    for (bit_field::itype i = 0; i < count; i++) {
        bit_field b({i});
        result.push_back(std::move(b));
    }

    return result;
}

bit_field calculate_sum_preincrement(const std::vector<bit_field>& b_vec) {
    bit_field result({0});

    for (auto it = b_vec.begin(); it != b_vec.end(); ++it) {
        result ^= *it;
    }

    return result;
}

bit_field calculate_sum_postincrement(const std::vector<bit_field>& b_vec) {
    bit_field result({0});

    for (auto it = b_vec.begin(); it != b_vec.end(); it++) {
        result ^= *it;
    }

    return result;
}

bit_field calculate_sum_range_value(const std::vector<bit_field>& b_vec) {
    bit_field result({0});

    for (auto b : b_vec) {
        result ^= b;
    }

    return result;
}

bit_field calculate_sum_range_reference(const std::vector<bit_field>& b_vec) {
    bit_field result({0});

    for (const auto& b : b_vec) {
        result ^= b;
    }

    return result;
}

bit_field calculate_sum_direct(const std::vector<bit_field>& b_vec) {
    bit_field result({0});
    size_t count = b_vec.size();

    for (size_t i = 0; i < count; i++) {
        result ^= b_vec[i];
    }

    return result;
}

bit_field calculate_sum_ctor(size_t count) {
    bit_field result({0});

    for (size_t i = 0; i < count; i++) {
        bit_field tmp({i});
        result ^= tmp;
    }

    return result;
}

bit_field calculate_sum_ctor_assign(size_t count) {
    bit_field result({0});

    for (size_t i = 0; i < count; i++) {
        bit_field tmp(sizeof(bit_field::itype));
        tmp = {i};
        result ^= tmp;
    }

    return result;
}

bit_field calculate_sum_assign(size_t count) {
    bit_field result({0});
    bit_field tmp({0});

    for (size_t i = 0; i < count; i++) {
        tmp = {i};
        result ^= tmp;
    }

    return result;
}

int main(int argc, char* argv[]) {
    bit_field a({2, 2, 2, 2});
    bit_field b({1, 1, 1, 1});
    bit_field scratch({0, 0, 0, 0});
    std::vector<bit_field> result;
    result.reserve(20);

    std::vector<bit_field> test_bit_fields = create_bitfield_vector(arr_len);

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
    assert(result[3] == result[4]);
    assert(result[2] == result[3]);

    std::cout << scratch << std::endl;

    {
        measure_time m("calculate_sum_direct");
        bit_field result = calculate_sum_direct(test_bit_fields);
        std::cout << result << std::endl;
    }
    {
        measure_time m("calculate_sum_preincrement");
        bit_field result = calculate_sum_preincrement(test_bit_fields);
        std::cout << result << std::endl;
    }
    {
        measure_time m("calculate_sum_postincrement");
        bit_field result = calculate_sum_postincrement(test_bit_fields);
        std::cout << result << std::endl;
    }
    {
        measure_time m("calculate_sum_range_reference");
        bit_field result = calculate_sum_range_reference(test_bit_fields);
        std::cout << result << std::endl;
    }
    {
        measure_time m("calculate_sum_range_value");
        bit_field result = calculate_sum_range_value(test_bit_fields);
        std::cout << result << std::endl;
    }
    {
        measure_time m("calculate_sum_ctor");
        bit_field result = calculate_sum_ctor(arr_len);
        std::cout << result << std::endl;
    }
    {
        measure_time m("calculate_sum_ctor_assign");
        bit_field result = calculate_sum_ctor_assign(arr_len);
        std::cout << result << std::endl;
    }
    {
        measure_time m("calculate_sum_assign");
        bit_field result = calculate_sum_assign(arr_len);
        std::cout << result << std::endl;
    }

    return 0;
}
