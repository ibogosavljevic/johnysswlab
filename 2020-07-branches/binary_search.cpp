#include "utils.h"

#include <chrono>
#include <iostream>
#include <vector>

#include "argparse.h"
#include "measure_time.h"


enum search_type {
   REGULAR,
   CONDITIONAL_MOVE,
   ARITHMETIC,
   UNKNOWN,
};

using namespace argparse;

bool parse_arguments(int argc, const char** argv, search_type& out_search_type, 
                    std::string& out_search_type_string, int& out_searches, 
                    int& out_length, bool& out_with_prefetching) {
    std::vector<std::pair<std::string, search_type>> search_type_map = {
         { "regular", search_type::REGULAR },
         { "cmov", search_type::CONDITIONAL_MOVE},
         { "arithmetic", search_type::ARITHMETIC },
    };
    ArgumentParser parser("binary_search", "Binary Search algorithm for different kind of branch avoidance");

    parser.add_argument("-t", "--test", "Name of the test to run", true);
    parser.add_argument("-s", "--searches", "Number of searches to perform", false);
    parser.add_argument("-l", "--length", "Array length", false);
    parser.add_argument("-p", "--prefetching", "If prefetching should be enabled or not", false);
    
    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return false;
    }
    
    if (parser.exists("t")) {
        out_search_type = search_type::UNKNOWN;
        std::string test_name = parser.get<std::string>("t");
        for (const auto& param: search_type_map) {
            if (param.first == test_name) {
                out_search_type = param.second;
                out_search_type_string = param.first;
                break;
            }
        }
        if (out_search_type == search_type::UNKNOWN) {
            std::cout << "Unknown value for --test\n";
            return false;
        }   
    } else {
        std::cout << "Parameter --test needed. Possible values for paramter: " << "\n";
        for (const auto& param: search_type_map) {
            std::cout << "--test " << param.first << "\n";
        }
        return false;
    }

    if (parser.exists("s")) {
        int searches_cmdl = parser.get<int>("s");
        if (searches_cmdl > 0 && searches_cmdl <= 10000000) {
            out_searches = searches_cmdl;
        }
    }

    if (parser.exists("l")) {
        int arr_len_cmdl = parser.get<int>("l");
        if (arr_len_cmdl >= 1000 && arr_len_cmdl <= 100000000) {
            out_length = arr_len_cmdl;
        }
    }

    if (parser.exists("p")) {
        out_with_prefetching = parser.get<bool>("p");
    }

    return true;
}

template <bool with_prefetching, search_type st>
int binary_search(int* array, int number_of_elements, int key) {
    int low = 0, high = number_of_elements-1, mid;
    while(low <= high) {
        mid = (low + high)/2;
        if (with_prefetching) {
            // low path
            __builtin_prefetch (&array[(mid + 1 + high)/2], 0, 1);
            // high path
            __builtin_prefetch (&array[(low + mid - 1)/2], 0, 1);
        }

        if (st == search_type::REGULAR) {
            if(array[mid] < key)
                low = mid + 1; 
            else if(array[mid] == key)
                return mid;
            else
                high = mid-1;
        }

        if (st == search_type::CONDITIONAL_MOVE) {
            int middle = array[mid];
            if (middle == key) {
                    return mid;
                }

            int new_low = mid + 1;
            int new_high = mid - 1;
#if defined(__x86_64)
            __asm__ (
                "cmp %[array_middle], %[key];"
                "cmovae %[new_low], %[low];"
                "cmovb %[new_high], %[high];"
                : [low] "+&r"(low), [high] "+&r"(high)
                : [new_low] "g"(new_low), [new_high] "g"(new_high), [array_middle] "g"(middle), [key] "g"(key)
                : "cc"
            );
#elif defined(__MIPSEL)
            __asm__ (
                "sub $8, %[array_middle], %[key];"
                "sra $8, $8, 31;"
                "movn %[low], %[new_low], $8;"
                "movz %[high], %[new_high], $8;"
                : [low] "+&r"(low), [high] "+&r"(high)
                : [new_low] "r"(new_low), [new_high] "r"(new_high), [array_middle] "r"(middle), [key] "r"(key)
                : "cc", "$8"
            );

#else
#error Implementation missing
#endif
        }

        if (st == search_type::ARITHMETIC) {
            int middle = array[mid];
            if (middle == key) {
                return mid;
            }

            int new_low = mid + 1;
            int new_high = mid - 1;
            int condition = array[mid] < key;
            int condition_true_mask = -condition;
            int condition_false_mask = -(1 - condition);

            low += condition_true_mask & (new_low - low);
            high += condition_false_mask & (new_high - high); 

        }
    }
    return -1;
}

int main(int argc, const char* argv[]) {
    bool prefetching = false;
    std::vector<int> index_array(1);
    int len = 4000000;
    int* input_array;
    int searches = len;
    search_type st = search_type::UNKNOWN;
    std::string st_name;

    bool parse_result = parse_arguments(argc, argv, st, st_name, searches, len, prefetching);
    if (!parse_result) {
        std::cout << "Bad args\n";
        return -1;
    }

    std::cout << "Binary search: \n";
    std::cout << "Search type: " << st_name << std::endl;
    std::cout << "Working set = " << len << "\n";
    std::cout << "Software prefetching = " << (prefetching ? "on" : "off") << "\n";
    std::cout << "Number of searches = " << searches << "\n";

    index_array = create_random_array<int>(searches, 0, len);
    input_array = (int*) malloc(sizeof(int) * len);
    generate_random_growing_array(input_array, len);

    {
        measure_time m("Binary search algorithm");
        if (prefetching) {
            if (st == search_type::REGULAR ) {
                for (int i = 0; i < searches; i++) {
                    binary_search<true, search_type::REGULAR>(input_array, len, input_array[index_array[i]]);
                }
            } else if (st == search_type::CONDITIONAL_MOVE) {
                for (int i = 0; i < searches; i++) {
                    binary_search<true, search_type::CONDITIONAL_MOVE>(input_array, len, input_array[index_array[i]]);
                }
            } else if (st == search_type::ARITHMETIC) {
                for (int i = 0; i < searches; i++) {
                    binary_search<true, search_type::ARITHMETIC>(input_array, len, input_array[index_array[i]]);
                }
            }
        }
        else {
            if (st == search_type::REGULAR ) {
                for (int i = 0; i < searches; i++) {
                    binary_search<false, search_type::REGULAR>(input_array, len, input_array[index_array[i]]);
                }
            } else if (st == search_type::CONDITIONAL_MOVE) {
                for (int i = 0; i < searches; i++) {
                    binary_search<false, search_type::CONDITIONAL_MOVE>(input_array, len, input_array[index_array[i]]);
                }
            } else if (st == search_type::ARITHMETIC) {
                for (int i = 0; i < searches; i++) {
                    binary_search<false, search_type::ARITHMETIC>(input_array, len, input_array[index_array[i]]);
                }
            }
        }
    }

    free(input_array);

    return 0;
}
