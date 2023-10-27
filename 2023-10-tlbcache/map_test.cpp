
#include "oa_hash_map.h"
#include <unordered_map>
#include <cstdint>
#include <iostream>
#include <vector>
#include <likwid.h>

struct test_struct {
    int val1;
    int val2;
    int val3;
    
    test_struct(int v1, int v2, int v3) : val1(v1), val2(v2), val3(v3) {}

    test_struct(const test_struct& from) : val1(from.val1), val2(from.val2), val3(from.val3) {}

    bool operator==(const test_struct& oth) {
        return val1 == oth.val1 && val2 == oth.val2 && val3 == oth.val3;
    }
};

#define EXPECT_TRUE(v) { if (!(v)) std::cout << "At line " << __LINE__ << " expression " << #v << " returned false\n";   }
#define EXPECT_FALSE(v) { if ((v)) std::cout << "At line " << __LINE__ << " expression " << #v << " returned true\n";   }
#define EXPECT_EQUAL(a,b) {if (!((a) == (b))) { std::cout << "At line " << __LINE__ << " expression " << #a << " and " << #b << " not equal\n"; __builtin_trap(); } }

enum operation_t {
    GET,
    TRY_EMPLACE,
    EMPLACE,
    REMOVE
};

std::vector<std::tuple<operation_t, int64_t, test_struct>> generate_test_data(size_t num_keys, size_t num_operations) {
    std::vector<std::tuple<operation_t, int64_t, test_struct>> result;
    result.reserve(num_operations);
    for (size_t i = 0; i < num_operations; i++) {
        // Generate a key
        int64_t key = (rand() % num_keys) * 16;
        operation_t operation = static_cast<operation_t>(rand() % 4);
        test_struct data(i, key, operation);
        auto t = std::make_tuple(operation, key, data);
        result.push_back(t);
    }
    return result;
}

template<typename T>
void run_test2(std::string name, T& test_map, const std::vector<std::tuple<operation_t, int64_t, test_struct>>& test_data) {
    size_t found = 0;
    LIKWID_MARKER_START(name.c_str());
    for (int i = 0; i < test_data.size(); i++) {
        const std::tuple<operation_t, int64_t, test_struct>& test_case = test_data[i];
        int64_t key = std::get<1>(test_case);
        test_struct value = std::get<2>(test_case);

        switch(std::get<0>(test_case)) {
            case GET: {
                found += test_map.get(key) != nullptr;
                break;
            }
            case EMPLACE: {
                test_map.emplace(key, value);
                break;
            }
            case TRY_EMPLACE: {
                test_map.try_emplace(key, value.val1, value.val2, value.val3);
                break;
            }
            case REMOVE: {
                test_map.remove(key);
                break;
            }
        }
    }
    LIKWID_MARKER_STOP(name.c_str());
    printf("found = %zu\n", found);
}

template<size_t size>
void run_test(std::string name, bool use_large_pages) {
    auto test_data = generate_test_data(size / 2, 8*1024*1024);

    jsl::oa_hash_map<size, int64_t, test_struct> oe_map(use_large_pages);
    std::unordered_map<int64_t, test_struct> test_map;
    test_map.reserve(size);

    for (int i = 0; i < test_data.size(); i++) {
        std::tuple<operation_t, int64_t, test_struct>& test_case = test_data[i];
        int64_t key = std::get<1>(test_case);
        test_struct value = std::get<2>(test_case);

        switch(std::get<0>(test_case)) {
            case GET: {
                auto res1 = test_map.find(key);
                test_struct* res2 = oe_map.get(key);
                EXPECT_EQUAL(res1 != test_map.end(), res2 != nullptr);
                if (res2) {
                    EXPECT_EQUAL(res1->second, *res2);
                }
                break;
            }
            case EMPLACE: {
                test_map.erase(key);
                auto res1 = test_map.emplace(key, value);
                oe_map.emplace(key, value);
                break;
            }
            case TRY_EMPLACE: {
                auto res1 = test_map.emplace(key, value);
                std::pair<bool, test_struct*> res2 = oe_map.try_emplace(key, value.val1, value.val2, value.val3);
                EXPECT_EQUAL(res1.second, res2.first);
                EXPECT_EQUAL(res1.first->second, *(res2.second));
                break;
            }
            case REMOVE: {
                size_t v = test_map.erase(key);
                bool result1 = oe_map.remove(key);
                EXPECT_EQUAL(result1, v);
                break;
            }
        }
    }

    run_test2("oe_map_" + name, oe_map, test_data);

    printf("OE_MAP:\n%s", oe_map.get_statistics().c_str());
}


int main(int argc, char** argv) {
    bool use_large_pages = false;

    if (argc >= 2 && (std::string(argv[1]) == "--use_large_pages")) {
        use_large_pages = true;
    }

    printf("Using large pages = %s\n", use_large_pages ? "TRUE" : "FALSE");
    
    LIKWID_MARKER_INIT;

    run_test<8*1024>("8k", use_large_pages);
    run_test<64*1024>("64k", use_large_pages);
    run_test<512*1024>("512k", use_large_pages);
    run_test<4*1024*1024>("4M", use_large_pages);
    run_test<32*1024*1024>("32M", use_large_pages);

    LIKWID_MARKER_CLOSE;

    return 0;
}