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
        measure_time m("Surface all " + out);
        int surface_all = calculate_surface_all(rectangles);
        std::cout << "Surface all is " << surface_all << std::endl;
    } else {
        measure_time m("Surface visible " + out);
        int surface_visible = calculate_surface_visible(rectangles);
        std::cout << "Surface visible is " << surface_visible << std::endl;
    }
}

static constexpr int size = 20 * 1024 * 1024;

void measure_class_padding(what_to_measure_e w) {
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

    measure<rectangle<0, 28>>(size, w);
    measure<rectangle<1, 28>>(size, w);
    measure<rectangle<2, 28>>(size, w);
    measure<rectangle<4, 28>>(size, w);
    measure<rectangle<8, 28>>(size, w);

    measure<rectangle<0, 36>>(size, w);
    measure<rectangle<2, 36>>(size, w);
    measure<rectangle<4, 36>>(size, w);
    measure<rectangle<8, 36>>(size, w);
    measure<rectangle<12, 36>>(size, w);
    measure<rectangle<16, 36>>(size, w);

    measure<rectangle<0, 44>>(size, w);
    measure<rectangle<2, 44>>(size, w);
    measure<rectangle<4, 44>>(size, w);
    measure<rectangle<8, 44>>(size, w);
    measure<rectangle<16, 44>>(size, w);
    measure<rectangle<24, 44>>(size, w);

    measure<rectangle<0, 60>>(size, w);
    measure<rectangle<2, 60>>(size, w);
    measure<rectangle<4, 60>>(size, w);
    measure<rectangle<8, 60>>(size, w);
    measure<rectangle<16, 60>>(size, w);
    measure<rectangle<24, 60>>(size, w);
    measure<rectangle<32, 60>>(size, w);

    measure<rectangle<0, 76>>(size, w);
    measure<rectangle<4, 76>>(size, w);
    measure<rectangle<8, 76>>(size, w);
    measure<rectangle<16, 76>>(size, w);
    measure<rectangle<24, 76>>(size, w);
    measure<rectangle<32, 76>>(size, w);
    measure<rectangle<48, 76>>(size, w);

    measure<rectangle<0, 108>>(size, w);
    measure<rectangle<4, 108>>(size, w);
    measure<rectangle<8, 108>>(size, w);
    measure<rectangle<16, 108>>(size, w);
    measure<rectangle<32, 108>>(size, w);
    measure<rectangle<48, 108>>(size, w);
    measure<rectangle<64, 108>>(size, w);

    measure<rectangle<0, 140>>(size, w);
    measure<rectangle<4, 140>>(size, w);
    measure<rectangle<8, 140>>(size, w);
    measure<rectangle<16, 140>>(size, w);
    measure<rectangle<32, 140>>(size, w);
    measure<rectangle<48, 140>>(size, w);
    measure<rectangle<64, 140>>(size, w);
    measure<rectangle<96, 140>>(size, w);
}

void measure_class_size(what_to_measure_e w) {
    measure<rectangle<0, 17>>(size, w);

    measure<rectangle<0, 18>>(size, w);

    measure<rectangle<0, 19>>(size, w);

    measure<rectangle<0, 20>>(size, w);
    measure<rectangle<0, 22>>(size, w);
    measure<rectangle<0, 24>>(size, w);

    measure<rectangle<0, 28>>(size, w);
    measure<rectangle<0, 36>>(size, w);
    measure<rectangle<0, 44>>(size, w);
    measure<rectangle<0, 60>>(size, w);

    measure<rectangle<0, 76>>(size, w);
    measure<rectangle<0, 108>>(size, w);
    measure<rectangle<0, 140>>(size, w);
}

int main(int argc, char** argv) {
    measure_class_size(SURFACE_VISIBLE);
    // measure_class_padding(SURFACE_VISIBLE);
}