
#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include "common/argparse.h"
#include "measure_time.h"
#include "objects.h"
#include "polymorphic_vector.h"

enum array_type_e {
    POINTER_ARRAY,
    DOD_ARRAY,
    POLYMORPHIC_ARRAY,
    VARIANT_ARRAY,
    VARIANT_VISITOR_ARRAY
};

using namespace argparse;

bool parse_args(int argc,
                const char* argv[],
                array_type_e& out_array_type,
                bool& out_shuffle) {
    ArgumentParser parser("test123", "123");

    parser.add_argument(
        "-a", "--array",
        "Array type (pointer, dod, polymorphic, variant, visitor)", true);
    parser.add_argument("-s", "--shuffle", "Shuffle array", false);

    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return false;
    }

    if (parser.exists("a")) {
        std::string array_type = parser.get<std::string>("a");
        std::cout << "Array type : " << array_type << std::endl;
        if (array_type == "pointer") {
            out_array_type = array_type_e::POINTER_ARRAY;
        } else if (array_type == "dod") {
            out_array_type = array_type_e::DOD_ARRAY;
        } else if (array_type == "polymorphic") {
            out_array_type = array_type_e::POLYMORPHIC_ARRAY;
        } else if (array_type == "variant") {
            out_array_type = array_type_e::VARIANT_ARRAY;
        } else if (array_type == "visitor") {
            out_array_type = array_type_e::VARIANT_VISITOR_ARRAY;
        } else {
            std::cout << "Unknown value for --array\n";
            return false;
        }
    } else {
        return false;
    }

    out_shuffle = false;

    if (parser.exists("s")) {
        out_shuffle = parser.get<bool>("s");
    }

    std::cout << "Shuffle type: " << out_shuffle << std::endl;

    return true;
}

object* to_base(std::variant<circle, line, rectangle, monster>& v) {
    if (std::holds_alternative<circle>(v)) {
        return &std::get<circle>(v);
    } else if (std::holds_alternative<line>(v)) {
        return &std::get<line>(v);
    } else if (std::holds_alternative<rectangle>(v)) {
        return &std::get<rectangle>(v);
    } else if (std::holds_alternative<monster>(v)) {
        return &std::get<monster>(v);
    } else {
        return nullptr;
    }
}

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

int main(int argc, const char* argv[]) {
    constexpr int arr_len = 20 * 1024 * 1024;
    bitmap b(1024, 768);
    array_type_e array_type;
    bool shuffle;

    if (!parse_args(argc, argv, array_type, shuffle)) {
        std::cout << "Bad arguments\n";
        return -1;
    }

    if (array_type == array_type_e::POLYMORPHIC_ARRAY) {
        polymorphic_vector<object, circle, rectangle, line, monster> v;
        v.reserve(4 * arr_len);

        for (int i = 0; i < arr_len; i++) {
            v.emplace_back<circle>(point(20, 20), 10);
        }
        for (int i = 0; i < arr_len; i++) {
            v.emplace_back<line>(point(0, 0), point(10, 10));
        }
        for (int i = 0; i < arr_len; i++) {
            v.emplace_back<rectangle>(point(0, 0), point(10, 10));
        }
        for (int i = 0; i < arr_len; i++) {
            v.emplace_back<monster>();
        }

        if (shuffle) {
            v.shuffle();
        }

        uint64_t pixels_drawn = 0;
        {
            measure_time m("Polymorphic vector: draw");
            for (int i = 0; i < arr_len * 4; i++) {
                pixels_drawn += v.get(i)->draw(b);
            }
        }

        std::cout << "Pixels drawn: " << pixels_drawn << std::endl;

        {
            measure_time m("Polymorphic vector: count virtual");
            unsigned int count = 0;
            for (int i = 0; i < arr_len * 4; i++) {
                count += v.get(i)->get_id();
            }
            std::cout << "Count virtual = " << count << std::endl;
        }

        {
            measure_time m("Polymorphic vector: count non-virtual");
            unsigned int count = 0;
            for (int i = 0; i < arr_len * 4; i++) {
                count += v.get(i)->get_id2();
            }
            std::cout << "Count non-virtual = " << count << std::endl;
        }

    } else if (array_type == array_type_e::VARIANT_ARRAY) {
        std::vector<std::variant<circle, line, rectangle, monster>> q;

        q.reserve(arr_len * 4);

        for (int i = 0; i < arr_len; i++) {
            q.push_back(circle(point(20, 20), 10));
        }
        for (int i = 0; i < arr_len; i++) {
            q.push_back(line(point(0, 0), point(10, 10)));
        }
        for (int i = 0; i < arr_len; i++) {
            q.push_back(rectangle(point(0, 0), point(10, 10)));
        }
        for (int i = 0; i < arr_len; i++) {
            q.push_back(monster());
        }

        if (shuffle) {
            std::random_shuffle(q.begin(), q.end());
        }

        {
            measure_time m("Variant array: draw");
            uint64_t pixels_drawn = 0;
            for (int i = 0; i < arr_len * 4; i++) {
                to_base(q[i])->draw(b);
            }
            std::cout << "Pixels drawn " << pixels_drawn << std::endl;
        }

        {
            measure_time m("Variant vector: count virtual");
            unsigned int count = 0;
            for (int i = 0; i < arr_len * 4; i++) {
                count += to_base(q[i])->get_id();
            }
            std::cout << "Count virtual " << count << std::endl;
        }

        {
            measure_time m("Variant vector: count non-virtual");
            unsigned int count = 0;
            for (int i = 0; i < arr_len * 4; i++) {
                count += to_base(q[i])->get_id2();
            }
            std::cout << "Count virtual non-virtual " << count << std::endl;
        }

    } else if (array_type == array_type_e::VARIANT_VISITOR_ARRAY) {
        std::vector<std::variant<circle, line, rectangle, monster>> q;

        q.reserve(arr_len * 4);

        for (int i = 0; i < arr_len; i++) {
            q.push_back(circle(point(20, 20), 10));
        }
        for (int i = 0; i < arr_len; i++) {
            q.push_back(line(point(0, 0), point(10, 10)));
        }
        for (int i = 0; i < arr_len; i++) {
            q.push_back(rectangle(point(0, 0), point(10, 10)));
        }
        for (int i = 0; i < arr_len; i++) {
            q.push_back(monster());
        }

        if (shuffle) {
            std::random_shuffle(q.begin(), q.end());
        }

        {
            measure_time m("Variant visitor array: draw");
            uint64_t pixels_drawn = 0;
            for (int i = 0; i < arr_len * 4; i++) {
                std::visit(overloaded{[&](auto& v) { v.draw(b); }}, q[i]);
            }
            std::cout << "Pixels drawn " << pixels_drawn << std::endl;
        }

        {
            measure_time m("Variant visitor vector: count virtual");
            unsigned int count = 0;
            for (int i = 0; i < arr_len * 4; i++) {
                count += std::visit(
                    overloaded{[&](auto& v) { return v.get_id(); }}, q[i]);
            }
            std::cout << "Count virtual " << count << std::endl;
        }

        {
            measure_time m("Variant visitor vector: count non-virtual");
            unsigned int count = 0;
            for (int i = 0; i < arr_len * 4; i++) {
                count += std::visit(
                    overloaded{[&](auto& v) { return v.get_id2(); }}, q[i]);
            }
            std::cout << "Count virtual non-virtual" << count << std::endl;
        }

    } else if (array_type == array_type_e::DOD_ARRAY) {
        std::vector<circle> v1;
        std::vector<line> v2;
        std::vector<rectangle> v3;
        std::vector<monster> v4;

        v1.reserve(arr_len);
        v2.reserve(arr_len);
        v3.reserve(arr_len);
        v4.reserve(arr_len);

        for (int i = 0; i < arr_len; i++) {
            v1.emplace_back(point(20, 20), 10);
            v2.emplace_back(point(0, 0), point(10, 10));
            v3.emplace_back(point(0, 0), point(10, 10));
            v4.emplace_back();
        }

        if (shuffle) {
            std::random_shuffle(v1.begin(), v1.end());
            std::random_shuffle(v2.begin(), v2.end());
            std::random_shuffle(v3.begin(), v3.end());
            std::random_shuffle(v4.begin(), v4.end());
        }

        {
            measure_time m("DOD vector: draw");
            uint64_t pixels_drawn = 0;
            for (int i = 0; i < arr_len; i++) {
                pixels_drawn += v1[i].draw(b);
            }
            for (int i = 0; i < arr_len; i++) {
                pixels_drawn += v2[i].draw(b);
            }
            for (int i = 0; i < arr_len; i++) {
                pixels_drawn += v3[i].draw(b);
            }
            for (int i = 0; i < arr_len; i++) {
                pixels_drawn += v4[i].draw(b);
            }
            std::cout << "Pixels drawn " << pixels_drawn << std::endl;
        }

        {
            measure_time m("DOD vector: count virtual");
            unsigned int count = 0;
            for (int i = 0; i < arr_len; i++) {
                count += v1[i].get_id();
            }
            for (int i = 0; i < arr_len; i++) {
                count += v2[i].get_id();
            }
            for (int i = 0; i < arr_len; i++) {
                count += v3[i].get_id();
            }
            for (int i = 0; i < arr_len; i++) {
                count += v4[i].get_id();
            }

            std::cout << "Count virtual " << count << std::endl;
        }

        {
            measure_time m("DOD vector: count non-virtual");
            unsigned int count = 0;
            for (int i = 0; i < arr_len; i++) {
                count += v1[i].get_id2();
            }
            for (int i = 0; i < arr_len; i++) {
                count += v2[i].get_id2();
            }
            for (int i = 0; i < arr_len; i++) {
                count += v3[i].get_id2();
            }
            for (int i = 0; i < arr_len; i++) {
                count += v4[i].get_id2();
            }

            std::cout << "Count non-virtual " << count << std::endl;
        }
    } else if (array_type == array_type_e::POINTER_ARRAY) {
        std::vector<object*> q;

        q.reserve(arr_len * 4);

        for (int i = 0; i < arr_len; i++) {
            q.push_back(new circle(point(20, 20), 10));
        }
        for (int i = 0; i < arr_len; i++) {
            q.push_back(new line(point(0, 0), point(10, 10)));
        }
        for (int i = 0; i < arr_len; i++) {
            q.push_back(new rectangle(point(0, 0), point(10, 10)));
        }
        for (int i = 0; i < arr_len; i++) {
            q.push_back(new monster());
        }

        if (shuffle) {
            std::random_shuffle(q.begin(), q.end());
        }

        {
            measure_time m("Array of pointers: draw");
            uint64_t pixels_drawn = 0;
            for (int i = 0; i < arr_len * 4; i++) {
                pixels_drawn += q[i]->draw(b);
            }
            std::cout << "Pixels drawn " << pixels_drawn << std::endl;
        }

        {
            measure_time m("Array of pointers: count virtual");
            unsigned int count = 0;
            for (int i = 0; i < arr_len * 4; i++) {
                count += q[i]->get_id();
            }
            std::cout << "Count virtual " << count << std::endl;
        }

        {
            measure_time m("Array of pointers: count non-virtual");
            unsigned int count = 0;
            for (int i = 0; i < arr_len * 4; i++) {
                count += q[i]->get_id2();
            }
            std::cout << "Count non-virtual " << count << std::endl;
        }
    }

    return 0;
}