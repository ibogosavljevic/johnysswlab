#include <algorithm>
#include "common/argparse.h"
#include "measure_time.h"
#include "multivector.h"
#include "objects.h"
#include "polymorphic_vector.h"

#define SORT_TYPES \
    X(ONE_TYPE)    \
    X(SORTED)      \
    X(RANDOM)      \
    X(TAKING_TURNS)

enum sort_type_t {
#define X(t) t,
    SORT_TYPES
#undef X
};

std::string to_string(sort_type_t t) {
    switch (t) {
#define X(t) \
    case t:  \
        return std::string(#t);
        SORT_TYPES
#undef X
        default:
            return "Unknown";
    };
    return "Unreachable";
}

enum test_type_e {
    INITIAL,
    DATA_CACHE,
    COMPILER_OPTIMIZATIONS,
    JUMP_MISPREDICTIONS,
    CACHE_EVICTIONS,
};

using namespace argparse;

bool parse_args(int argc,
                const char* argv[],
                size_t& out_size,
                test_type_e& out_test_type) {
    ArgumentParser parser("test123", "123");

    parser.add_argument("-s", "--size", "Size of the input arrays (s, m, l)",
                        true);
    parser.add_argument("-t", "--type", "Type of test", true);

    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return false;
    }

    if (parser.exists("s")) {
        std::string size = parser.get<std::string>("s");
        if (size == "s") {
            out_size = 1024;
        } else if (size == "m") {
            out_size = 512 * 1024;
        } else if (size == "l") {
            out_size = 20 * 1024 * 1024;
        } else {
            std::cout << "Unknown value for --size\n";
            return false;
        }
        std::cout << "Array size : " << out_size << std::endl;
    } else {
        return false;
    }

    if (parser.exists("t")) {
        std::string t = parser.get<std::string>("t");
        if (t == "initial") {
            out_test_type = INITIAL;
        } else if (t == "datacache") {
            out_test_type = DATA_CACHE;
        } else if (t == "compileropt") {
            out_test_type = COMPILER_OPTIMIZATIONS;
        } else if (t == "jumpmispred") {
            out_test_type = JUMP_MISPREDICTIONS;
        } else if (t == "cacheevict") {
            out_test_type = CACHE_EVICTIONS;
        } else {
            std::cout
                << "Unknown option for type. allowed options are: initial, "
                   "datacache, compileropt, jumpmispred, cacheevict.\n";
            return false;
        }
    }

    return true;
}

point get_random_point(int width, int height) {
    return point(std::rand() % width, std::rand() % height);
}

template <typename T>
void fill_container(T& container,
                    int count,
                    sort_type_t s,
                    int width,
                    int height) {
    container.reserve(count);
    if (s == ONE_TYPE) {
        int type = std::rand() % 4;
        switch (type) {
            case 0:
                for (int i = 0; i < count; i++) {
                    container.template emplace_back<circle>(
                        get_random_point(width, height), 50);
                }
                break;
            case 1:
                for (int i = 0; i < count; i++) {
                    container.template emplace_back<line>(
                        get_random_point(width, height),
                        get_random_point(width, height));
                }
                break;
            case 2:
                for (int i = 0; i < count; i++) {
                    container.template emplace_back<rectangle>(
                        get_random_point(width, height),
                        get_random_point(width, height));
                }
                break;
            case 3:
                for (int i = 0; i < count; i++) {
                    container.template emplace_back<monster>();
                }
                break;
        }
    } else if (s == SORTED) {
        int count_per_type = count / 4;
        for (int i = 0; i < count_per_type; i++) {
            container.template emplace_back<circle>(
                get_random_point(width, height), 50);
        }

        for (int i = count_per_type; i < 2 * count_per_type; i++) {
            container.template emplace_back<line>(
                get_random_point(width, height),
                get_random_point(width, height));
        }

        for (int i = 2 * count_per_type; i < 3 * count_per_type; i++) {
            container.template emplace_back<rectangle>(
                get_random_point(width, height),
                get_random_point(width, height));
        }

        for (int i = 3 * count_per_type; i < count; i++) {
            container.template emplace_back<monster>();
        }
    } else if (s == TAKING_TURNS) {
        for (int i = 0; i < count; i++) {
            int type = i % 4;
            switch (type) {
                case 0:
                    container.template emplace_back<circle>(
                        get_random_point(width, height), 50);
                    break;
                case 1:
                    container.template emplace_back<line>(
                        get_random_point(width, height),
                        get_random_point(width, height));
                    break;
                case 2:
                    container.template emplace_back<rectangle>(
                        get_random_point(width, height),
                        get_random_point(width, height));
                    break;
                case 3:
                    container.template emplace_back<monster>();
                    break;
            }
        }
    } else if (s == RANDOM) {
        for (int i = 0; i < count; i++) {
            int type = std::rand() % 4;
            switch (type) {
                case 0:
                    container.template emplace_back<circle>(
                        get_random_point(width, height), 50);
                    break;
                case 1:
                    container.template emplace_back<line>(
                        get_random_point(width, height),
                        get_random_point(width, height));
                    break;
                case 2:
                    container.template emplace_back<rectangle>(
                        get_random_point(width, height),
                        get_random_point(width, height));
                    break;
                case 3:
                    container.template emplace_back<monster>();
                    break;
            }
        }
    }
}

template <typename T>
int perform_swap(T& container,
                 std::vector<int>& swap_array,
                 int start,
                 int count) {
    int container_size = container.size();

    int performed_swaps = 0;

    int end = std::min<int>(start + count, swap_array.size());
    for (int i = start; i < end; i++, performed_swaps++) {
        std::swap(container[0], container[swap_array[i]]);
    }

    return performed_swaps;
}

template <class Functor>
void measure_cache_performance(int arr_len, Functor fn) {
    polymorphic_vector<object, circle, line, rectangle, monster> pv;
    std::vector<object*> ov(arr_len);
    std::vector<int> swap_array(arr_len);

    std::iota(swap_array.begin(), swap_array.end(), 0);
    std::random_shuffle(swap_array.begin(), swap_array.end());

    fill_container(pv, arr_len, ONE_TYPE, 640, 480);
    int repeat_count = 20 * 1024 * 1024 / arr_len;

    for (int i = 0; i < arr_len; i++) {
        ov[i] = pv.get(i);
    }

    int tmp;
    {
        measure_time m("Completely sorted");

        for (int s = 0; s < repeat_count; s++) {
            tmp = 0;
            for (int i = 0; i < arr_len; i++) {
                tmp += fn(ov[i]);
            }
        }
    }

    int goal_swaps = 256;
    int current_swaps = 0;
    char tmp_str[64];
    do {
        current_swaps += perform_swap(ov, swap_array, current_swaps,
                                      goal_swaps - current_swaps);
        std::sprintf(tmp_str, "%d swaps", current_swaps);

        {
            measure_time m(tmp_str);
            int tmp_2;
            for (int s = 0; s < repeat_count; s++) {
                tmp_2 = 0;
                for (int i = 0; i < ov.size(); i++) {
                    tmp_2 += fn(ov[i]);
                }
            }

            if (tmp != tmp_2) {
                std::cout << "Error: " << std::endl;
            }
        }

        goal_swaps += goal_swaps;
    } while ((goal_swaps / 2) < arr_len);
}

void measure_inlining_performance(int arr_len) {
    polymorphic_vector<object, circle, line, rectangle, monster> pv;
    fill_container(pv, arr_len, RANDOM, 640, 480);

    int count = 0;
    {
        measure_time m("Inlining performance");
        for (int i = 0; i < arr_len; i++) {
            object* o = pv.get(i);
            if (o->is_visible()) {
                count += o->get_id3();
            }
        }

        std::cout << "Count is " << count << std::endl;
    }
}

void measure_jump_misprediction_performance(int arr_len,
                                            sort_type_t sort_type) {
    polymorphic_vector<object, circle, line, rectangle, monster> pv;
    fill_container(pv, arr_len, sort_type, 640, 480);
    bitmap b(640, 480);

    {
        measure_time m("Function get_id()");

        int count = 0;
        for (int i = 0; i < arr_len; i++) {
            object* o = pv.get(i);
            count += o->get_id();
        }
        std::cout << "Count is " << count << std::endl;
    }
}

void measure_jump_misprediction_performance(int arr_len) {
    std::cout << "SORTED" << std::endl;
    measure_jump_misprediction_performance(arr_len, SORTED);
    std::cout << "TAKING_TURNS" << std::endl;
    measure_jump_misprediction_performance(arr_len, TAKING_TURNS);
    std::cout << "RANDOM" << std::endl;
    measure_jump_misprediction_performance(arr_len, RANDOM);
}

void measure_instruction_cache_performance(int arr_len, sort_type_t sort_type) {
    polymorphic_vector<object, circle, line, rectangle, monster> pv;
    fill_container(pv, arr_len, sort_type, 640, 480);
    bitmap b(640, 480);
    std::vector<int> random_nums(arr_len);

    int n = 0;
    std::generate(random_nums.begin(), random_nums.end(), [&n] {
        n++;
        if (n == 100) {
            n = 0;
        };
        return n;
    });
    std::random_shuffle(random_nums.begin(), random_nums.end());
    for (int repeat = 0; repeat < 10; repeat++) {
        measure_time m("long_virtual_function " + to_string(sort_type));

        int count = 0;
        for (int i = 0; i < arr_len - 100; i++) {
            object* o = pv.get(i);
            count += o->long_virtual_function(random_nums, i, 100);
        }
        std::cout << "Count is " << count << std::endl;
    }
}

void measure_instruction_cache_performance(int arr_len) {
    measure_time_database<std::chrono::milliseconds>::get_instance()
        ->clear_database();
    std::cout << "SORTED" << std::endl;
    measure_instruction_cache_performance(arr_len, SORTED);
    std::cout << "TAKING_TURNS" << std::endl;
    measure_instruction_cache_performance(arr_len, TAKING_TURNS);
    measure_time_database<std::chrono::milliseconds>::get_instance()
        ->dump_database();
}

void measure_virtual_function_performance(int arr_len) {
#ifndef NOINLINE
    std::cout << "Uncomment NOINLINE in object.h for accurate measurements";
#endif

    jsl::multivector<circle, line, rectangle, monster> mv;
    fill_container(mv, arr_len, SORTED, 640, 480);
    std::vector<object*> object_vector;
    object_vector.reserve(arr_len);

    bitmap b(640, 480);

    mv.for_all([&object_vector](auto& o) { object_vector.push_back(&o); });

    {
        measure_time m("Small virtual function, direct call");
        int tmp = 0;

        mv.for_all([&tmp](auto& o) { tmp += o.get_id(); });
        std::cout << "Tmp = " << tmp << std::endl;
    }

    {
        measure_time m("Small virtual function, virtual call");
        int tmp = 0;

        for (auto& o : object_vector) {
            tmp += o->get_id();
        }
        std::cout << "Tmp = " << tmp << std::endl;
    }

    {
        measure_time m("Large virtual function, direct call");
        int tmp = 0;

        mv.for_all([&tmp, &b](auto& o) { tmp += o.draw(b); });
        std::cout << "Tmp = " << tmp << std::endl;
    }

    {
        measure_time m("Large virtual function, virtual call");
        int tmp = 0;

        for (auto& o : object_vector) {
            tmp += o->draw(b);
        }
        std::cout << "Tmp = " << tmp << std::endl;
    }
}

int main(int argc, const char** argv) {
    size_t out_size;
    test_type_e t;

    if (!parse_args(argc, argv, out_size, t)) {
        std::cout << "Bad arguments" << std::endl;
        return -1;
    }

    switch (t) {
        case INITIAL:
            measure_virtual_function_performance(out_size);
            break;
        case DATA_CACHE:
            measure_cache_performance(out_size,
                                      [](object* o) { return o->get_id(); });
            measure_cache_performance(out_size,
                                      [](object* o) { return o->get_id2(); });
            break;
        case COMPILER_OPTIMIZATIONS:
            measure_inlining_performance(out_size);
            break;
        case JUMP_MISPREDICTIONS:
            measure_jump_misprediction_performance(out_size);
            break;
        case CACHE_EVICTIONS:
            measure_instruction_cache_performance(out_size);
            break;
        default:
            std::cout << "Unreachable\n";
            break;
    }
    return 0;
}