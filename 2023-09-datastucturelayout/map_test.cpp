
#include "oa_hash_map.h"
#include "sc_hash_map.h"
#include <unordered_map>
#include <cstdint>
#include <iostream>
#include <vector>
#include <likwid.h>
#include <absl/container/flat_hash_map.h>

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
void run_test1(std::string name, T& test_map, const std::vector<std::tuple<operation_t, int64_t, test_struct>>& test_data) {
    LIKWID_MARKER_START(name.c_str());
    size_t found = 0;
    for (int i = 0; i < test_data.size(); i++) {
        const std::tuple<operation_t, int64_t, test_struct>& test_case = test_data[i];
        int64_t key = std::get<1>(test_case);
        test_struct value = std::get<2>(test_case);

        switch(std::get<0>(test_case)) {
            case GET: {
                auto res1 = test_map.find(key);
                found += test_map.find(key) != test_map.end();
                break;
            }
            case EMPLACE: {
                test_map.erase(key);
                test_map.emplace(key, value);
                break;
            }
            case TRY_EMPLACE: {
                test_map.emplace(key, value);
                break;
            }
            case REMOVE: {
                test_map.erase(key);
                break;
            }
        }
    }
    LIKWID_MARKER_STOP(name.c_str());

    printf("found = %zu\n", found);
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
void run_test(std::string name) {
    auto test_data = generate_test_data(size / 2, 8*1024*1024);

    jsl::oa_hash_map<size, int64_t, test_struct> oe_map;
    jsl::sc_hash_map<size, int64_t, test_struct> sc_map;
    std::unordered_map<int64_t, test_struct> test_map;
    absl::flat_hash_map<int64_t, test_struct> abs_map;
    test_map.reserve(size);
    abs_map.reserve(size);

    for (int i = 0; i < test_data.size(); i++) {
        std::tuple<operation_t, int64_t, test_struct>& test_case = test_data[i];
        int64_t key = std::get<1>(test_case);
        test_struct value = std::get<2>(test_case);

        switch(std::get<0>(test_case)) {
            case GET: {
                auto res1 = test_map.find(key);
                test_struct* res2 = oe_map.get(key);
                test_struct* res3 = sc_map.get(key);
                auto res4 = abs_map.find(key);
                EXPECT_EQUAL(res1 != test_map.end(), res2 != nullptr);
                EXPECT_EQUAL(res1 != test_map.end(), res3 != nullptr);
                EXPECT_EQUAL(res1 != test_map.end(), res4 != abs_map.end());
                if (res2) {
                    EXPECT_EQUAL(res1->second, *res2);
                    EXPECT_EQUAL(res1->second, *res3);
                    EXPECT_EQUAL(res1->second, res4->second);
                }
                break;
            }
            case EMPLACE: {
                test_map.erase(key);
                auto res1 = test_map.emplace(key, value);
                oe_map.emplace(key, value);
                sc_map.emplace(key, value);
                abs_map.erase(key);
                abs_map.emplace(key, value);
                break;
            }
            case TRY_EMPLACE: {
                auto res1 = test_map.emplace(key, value);
                std::pair<bool, test_struct*> res2 = oe_map.try_emplace(key, value.val1, value.val2, value.val3);
                std::pair<bool, test_struct*> res3 = sc_map.try_emplace(key, value.val1, value.val2, value.val3);
                auto res4 = abs_map.emplace(key, value);
                EXPECT_EQUAL(res1.second, res2.first);
                EXPECT_EQUAL(res1.first->second, *(res2.second));
                EXPECT_EQUAL(res1.second, res3.first);
                EXPECT_EQUAL(res1.first->second, *(res3.second));
                EXPECT_EQUAL(res1.first->second, res4.first->second);
                EXPECT_EQUAL(res1.second, res4.second);
                break;
            }
            case REMOVE: {
                size_t v = test_map.erase(key);
                bool result1 = oe_map.remove(key);
                bool result2 = sc_map.remove(key);
                size_t v2 = abs_map.erase(key);
                EXPECT_EQUAL(result1, v);
                EXPECT_EQUAL(result2, v);
                EXPECT_EQUAL(v2, v);
                break;
            }
        }
    }

    run_test1("std_unordered_map_" + name, test_map, test_data);
    run_test1("absl_flat_hash_map_" + name, abs_map, test_data);
    run_test2("sc_map_" + name, sc_map, test_data);
    run_test2("oe_map_" + name, oe_map, test_data);

    printf("OE_MAP:\n%s", oe_map.get_statistics().c_str());
    printf("SC_MAP\n%s", sc_map.get_statistics().c_str());
}


int main(int argc, char** argv) {
    static constexpr size_t hashmap_size = 16*1024*1024;
    {
        jsl::oa_hash_map<hashmap_size, int64_t, test_struct> oe_map;

        EXPECT_TRUE(oe_map.emplace(0, 1, 2, 3));
        EXPECT_TRUE(oe_map.emplace(1, 2, 2, 3));
        EXPECT_TRUE(oe_map.get(0)->val1 == 1);
        EXPECT_TRUE(oe_map.get(1)->val1 == 2);
        EXPECT_TRUE(oe_map.remove(0));
        EXPECT_FALSE(oe_map.remove(2));
        EXPECT_FALSE(oe_map.get(0));

        EXPECT_TRUE(oe_map.try_emplace(0, 2, 2, 3).first);
        EXPECT_TRUE(oe_map.emplace(0, 2, 2, 3));

    }

    {
        jsl::sc_hash_map<hashmap_size, int64_t, test_struct> sc_map;

        EXPECT_TRUE(sc_map.emplace(0, 1, 2, 3));
        EXPECT_TRUE(sc_map.emplace(1, 2, 2, 3));
        EXPECT_TRUE(sc_map.get(0)->val1 == 1);
        EXPECT_TRUE(sc_map.get(1)->val1 == 2);
        EXPECT_TRUE(sc_map.remove(0));
        EXPECT_FALSE(sc_map.remove(2));
        EXPECT_FALSE(sc_map.get(0));

        EXPECT_TRUE(sc_map.try_emplace(0, 2, 2, 3).first);
        EXPECT_TRUE(sc_map.emplace(0, 2, 2, 3));
        EXPECT_TRUE(sc_map.emplace(hashmap_size, 3, 2, 2));
        EXPECT_TRUE(sc_map.emplace((2*hashmap_size), 4, 2, 2));
        EXPECT_TRUE(sc_map.remove(0));
        EXPECT_TRUE(sc_map.remove(2*hashmap_size));
        EXPECT_TRUE(sc_map.get(hashmap_size));
        EXPECT_TRUE(sc_map.remove(hashmap_size));
    }



    LIKWID_MARKER_INIT;

    run_test<8*1024>("8k");
    run_test<64*1024>("64k");
    run_test<512*1024>("512k");
    run_test<4*1024*1024>("4M");
    run_test<32*1024*1024>("32M");

    LIKWID_MARKER_CLOSE;

    return 0;
}