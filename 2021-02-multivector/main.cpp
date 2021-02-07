#include "common/argparse.h"
#include "measure_time.h"
#include "multivector.h"
#include "objects.h"
#include "polymorphic_vector.h"

enum sort_type_t {
    SORTED,
    RANDOM,
    TAKING_TURNS,
};

using namespace argparse;

bool parse_args(int argc, const char* argv[], size_t& out_size) {
    ArgumentParser parser("test123", "123");

    parser.add_argument("-s", "--size", "Size of the input arrays (s, m, l)",
                        true);

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
    if (s == SORTED) {
        int count_per_type = count / 4;
        for (int i = 0; i < count_per_type; i++) {
            container.template emplace_back<circle>(
                get_random_point(width, height), 5);
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
                        get_random_point(width, height), 5);
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
    } else {
        for (int i = 0; i < count; i++) {
            int type = std::rand() % 4;
            switch (type) {
                case 0:
                    container.template emplace_back<circle>(
                        get_random_point(width, height), 5);
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
void perform_swap(T& container, int count) {
    int container_size = container.size();
    for (int i = 0; i < count; i++) {
        int random = std::rand() % container_size;
        std::swap(container[0], container[random]);
    }
}

template <class Functor>
void measure_cache_performance(int arr_len, Functor fn) {
    polymorphic_vector<object, circle, line, rectangle, monster> pv;
    std::vector<object*> ov(arr_len);

    fill_container(pv, arr_len, RANDOM, 640, 480);
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
        perform_swap(ov, goal_swaps - current_swaps);
        current_swaps = goal_swaps;
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
    } while ((goal_swaps / 20) < arr_len);
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
                count += o->get_id2();
            }
        }

        std::cout << "Count is " << count << std::endl;
    }
}

int main(int argc, const char** argv) {
    size_t out_size;

    if (!parse_args(argc, argv, out_size)) {
        std::cout << "Bad arguments" << std::endl;
        return -1;
    }

    // measure_cache_performance(out_size, [](object* o) { return o->get_id2();
    // });
    measure_inlining_performance(out_size);

    return 0;
    // polymorphic_vector<object, circle, line, rectangle, monster> mv;
    jsl::multivector<circle, line, rectangle, monster> mv;

    bitmap b(640, 480);

    fill_container(mv, out_size, TAKING_TURNS, 640, 480);
    {
        measure_time m("test");
        int tmp = 0;
        // mv.for_all([&tmp] (auto& o) { tmp += o.get_id(); });
        /*for (int i = 0; i < mv.size(); i++) {
            object* o = mv.get(i);
            tmp += o->get_id2();
        }*/
        /*for(auto& c: mv.get_vector<circle>()) {
            tmp += c.get_id();
        }
        for(auto& c: mv.get_vector<line>()) {
            tmp += c.get_id();
        }
        for(auto& c: mv.get_vector<rectangle>()) {
            tmp += c.get_id();
        }
        for(auto& c: mv.get_vector<monster>()) {
            tmp += c.get_id();
        }*/
        std::cout << "Tmp = " << tmp << std::endl;
    }

    return 0;
}