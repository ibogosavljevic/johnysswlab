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

    for (int i = 0; i < arr_len; i++) {
        ov[i] = pv.get(i);
    }

    int tmp = 0;
    {
        measure_time m("Completely sorted");

        for (int i = 0; i < arr_len; i++) {
            tmp += fn(ov[i]);
        }
    }

    int total_swaps = 256;
    char tmp_str[64];
    do {
        std::sprintf(tmp_str, "%d swaps", total_swaps);
        perform_swap(ov, total_swaps);

        {
            measure_time m(tmp_str);
            int tmp_2 = 0;
            for (int i = 0; i < ov.size(); i++) {
                tmp_2 += fn(ov[i]);
            }

            if (tmp != tmp_2) {
                std::cout << "Error: " << std::endl;
            }
        }

        total_swaps += total_swaps;
    } while ((total_swaps / 20) < arr_len);
}

static constexpr int ARR_LEN = 20 * 1024 * 1024;

int main(int argc, char* argv[]) {
    // polymorphic_vector<object, circle, line, rectangle, monster> mv;
    jsl::multivector<circle, line, rectangle, monster> mv;

    bitmap b(640, 480);

    measure_cache_performance(ARR_LEN, [](object* o) { return o->get_id2(); });

    return 0;

    fill_container(mv, ARR_LEN, TAKING_TURNS, 640, 480);
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