#include <iostream>
#include <vector>
#include "measure_time.h"

struct point {
    int x;
    int y;
    point(int m_x, int m_y) : x(m_x), y(m_y) {}
};

template <int pad1_size, int class_size>
class rectangle {
   public:
    rectangle(point p1, point p2, bool visible)
        : m_p1(p1), m_p2(p2), m_visible(visible) {}
    int surface() {
        int x = std::abs(m_p1.x - m_p2.x);
        int y = std::abs(m_p1.y - m_p2.y);
        return x * y;
    }
    bool is_visible() { return m_visible; }
    static int get_padding_size() { return pad1_size * sizeof(int); }
    static int get_p1_offset() {
        rectangle r(point(0, 0), point(0, 0), true);
        long offset = (char*)&(r.m_p1) - (char*)&r;
        return (int)(offset);
    }
    static int get_end_padding() {
        return sizeof(int) *
               (class_size - pad1_size - sizeof(bool) - 2 * sizeof(point));
    }

   private:
    bool m_visible;
    int m_padding1[pad1_size];
    point m_p1;
    point m_p2;
    int m_padding2[class_size - pad1_size - sizeof(bool) - 2 * sizeof(point)];
};

template <typename R>
int calculate_surface_visible(std::vector<R>& rectangles) {
    int sum = 0;
    for (int i = 0; i < rectangles.size(); i++) {
        if (rectangles[i].is_visible()) {
            sum += rectangles[i].surface();
        }
    }
    return sum;
}

template <typename R>
int calculate_surface_all(std::vector<R>& rectangles) {
    int sum = 0;
    for (int i = 0; i < rectangles.size(); i++) {
        sum += rectangles[i].surface();
    }
    return sum;
}

template <typename T>
void fill_rectangles(std::vector<T>& rectangles, int size) {
    rectangles.reserve(size);
    std::srand(size);

    for (int i = 0; i < size; ++i) {
        int rand = std::rand();
        bool visible = rand % 2;

        rectangles.emplace_back(point(rand % 257, rand % 709),
                                point(rand % 311, rand % 237), visible);
    }
}

enum what_to_measure_e { SURFACE_ALL, SURFACE_VISIBLE };

template <typename T>
void measure(int size, what_to_measure_e what_to_measure) {
    std::vector<T> rectangles;
    fill_rectangles(rectangles, size);

    // std::cout << "Class size is " << sizeof(T) << ", padding size = " <<
    // T::get_padding_size() << std::endl;
    std::string out = "Class size is ";
    out.append(std::to_string(sizeof(T)));
    out.append(", padding size = ");
    out.append(std::to_string(T::get_padding_size()));
    out.append(", p1 offset = ");
    out.append(std::to_string(T::get_p1_offset()));
    out.append(", end padding = ");
    out.append(std::to_string(T::get_end_padding()));
    out.append("\n");
    std::cout << out;
    if (what_to_measure == SURFACE_ALL) {
        for (int i = 0; i < 30; i++) {
            measure_time m("Surface all " + out);
            int surface_all = calculate_surface_all(rectangles);
            std::cout << "Surface all is " << surface_all << std::endl;
        }
    } else {
        for (int i = 0; i < 30; i++) {
            measure_time m("Surface visible " + out);
            int surface_visible = calculate_surface_visible(rectangles);
            std::cout << "Surface visible is " << surface_visible << std::endl;
        }
    }
}

static constexpr int size = 20 * 1024 * 1024;

// With class size 512, we can observe conflict misses
// for some padding value (on CLANG it was paddng 32)
void measure_cache_conflict_misses(what_to_measure_e w) {
    measure_time_database<std::chrono::milliseconds>::get_instance()
        ->clear_database();

    measure<rectangle<0, 140>>(size, w);
    measure<rectangle<4, 140>>(size, w);
    measure<rectangle<8, 140>>(size, w);
    measure<rectangle<16, 140>>(size, w);
    measure<rectangle<32, 140>>(size, w);
    measure<rectangle<48, 140>>(size, w);
    measure<rectangle<64, 140>>(size, w);
    measure<rectangle<96, 140>>(size, w);

    measure_time_database<std::chrono::milliseconds>::get_instance()
        ->dump_database();
}

void measure_class_padding(what_to_measure_e w) {
    measure_time_database<std::chrono::milliseconds>::get_instance()
        ->clear_database();

    measure<rectangle<0, 17>>(size, w);

    measure<rectangle<0, 18>>(size, w);
    measure<rectangle<1, 18>>(size, w);

    measure<rectangle<0, 19>>(size, w);
    measure<rectangle<1, 19>>(size, w);
    measure<rectangle<2, 19>>(size, w);

    measure<rectangle<0, 20>>(size, w);
    measure<rectangle<1, 20>>(size, w);
    measure<rectangle<2, 20>>(size, w);
    measure<rectangle<3, 20>>(size, w);

    measure<rectangle<0, 22>>(size, w);
    measure<rectangle<1, 22>>(size, w);
    measure<rectangle<2, 22>>(size, w);
    measure<rectangle<4, 22>>(size, w);

    measure<rectangle<0, 24>>(size, w);
    measure<rectangle<1, 24>>(size, w);
    measure<rectangle<2, 24>>(size, w);
    measure<rectangle<4, 24>>(size, w);
    measure<rectangle<6, 24>>(size, w);

    measure<rectangle<0, 26>>(size, w);
    measure<rectangle<1, 26>>(size, w);
    measure<rectangle<2, 26>>(size, w);
    measure<rectangle<4, 26>>(size, w);
    measure<rectangle<8, 26>>(size, w);

    measure<rectangle<0, 34>>(size, w);
    measure<rectangle<2, 34>>(size, w);
    measure<rectangle<4, 34>>(size, w);
    measure<rectangle<8, 34>>(size, w);
    measure<rectangle<12, 34>>(size, w);
    measure<rectangle<16, 34>>(size, w);

    measure<rectangle<0, 42>>(size, w);
    measure<rectangle<2, 42>>(size, w);
    measure<rectangle<4, 42>>(size, w);
    measure<rectangle<8, 42>>(size, w);
    measure<rectangle<16, 42>>(size, w);
    measure<rectangle<24, 42>>(size, w);

    measure<rectangle<0, 58>>(size, w);
    measure<rectangle<2, 58>>(size, w);
    measure<rectangle<4, 58>>(size, w);
    measure<rectangle<8, 58>>(size, w);
    measure<rectangle<16, 58>>(size, w);
    measure<rectangle<24, 58>>(size, w);
    measure<rectangle<32, 58>>(size, w);

    measure<rectangle<0, 74>>(size, w);
    measure<rectangle<4, 74>>(size, w);
    measure<rectangle<8, 74>>(size, w);
    measure<rectangle<16, 74>>(size, w);
    measure<rectangle<24, 74>>(size, w);
    measure<rectangle<32, 74>>(size, w);
    measure<rectangle<48, 74>>(size, w);

    measure<rectangle<0, 106>>(size, w);
    measure<rectangle<4, 106>>(size, w);
    measure<rectangle<8, 106>>(size, w);
    measure<rectangle<16, 106>>(size, w);
    measure<rectangle<32, 106>>(size, w);
    measure<rectangle<48, 106>>(size, w);
    measure<rectangle<64, 106>>(size, w);

    measure<rectangle<0, 138>>(size, w);
    measure<rectangle<4, 138>>(size, w);
    measure<rectangle<8, 138>>(size, w);
    measure<rectangle<16, 138>>(size, w);
    measure<rectangle<32, 138>>(size, w);
    measure<rectangle<48, 138>>(size, w);
    measure<rectangle<64, 138>>(size, w);
    measure<rectangle<96, 138>>(size, w);

    measure_time_database<std::chrono::milliseconds>::get_instance()
        ->dump_database();
}

void measure_class_size(what_to_measure_e w) {
    measure_time_database<std::chrono::milliseconds>::get_instance()
        ->clear_database();

    measure<rectangle<0, 17>>(size, w);  // 20

    measure<rectangle<0, 18>>(size, w);  // 24

    measure<rectangle<0, 19>>(size, w);  // 28

    measure<rectangle<0, 20>>(size, w);  // 32
    measure<rectangle<0, 22>>(size, w);  // 40
    measure<rectangle<0, 24>>(size, w);  // 48

    measure<rectangle<0, 26>>(size, w);  // 56
    // measure<rectangle<0, 28>>(size, w); // 64
    measure<rectangle<0, 34>>(size, w);  // 88
    // measure<rectangle<0, 36>>(size, w); // 96
    measure<rectangle<0, 42>>(size, w);  // 120
    // measure<rectangle<0, 44>>(size, w); // 128

    measure<rectangle<0, 58>>(size, w);  // 184
    // measure<rectangle<0, 60>>(size, w); // 192

    measure<rectangle<0, 74>>(size, w);  // 248
    // measure<rectangle<0, 76>>(size, w); // 256
    measure<rectangle<0, 106>>(size, w);  // 376
    // measure<rectangle<0, 108>>(size, w); // 384
    measure<rectangle<0, 138>>(size, w);  // 504
    // measure<rectangle<0, 140>>(size, w); // 512

    measure_time_database<std::chrono::milliseconds>::get_instance()
        ->dump_database();
}

int main(int argc, char** argv) {
    measure_class_size(SURFACE_ALL);
    measure_class_size(SURFACE_VISIBLE);

    measure_class_padding(SURFACE_VISIBLE);
    measure_class_padding(SURFACE_ALL);

    measure_cache_conflict_misses(SURFACE_VISIBLE);
}