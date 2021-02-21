#include <cmath>

//#define NOINLINE

#ifdef NOINLINE
#define ATTRNOINLINE __attribute__((noinline))
#else
#define ATTRNOINLINE
#endif

#define UNROLL_COUNT 20

#define VARIABLES       \
    V(1, a* a* a)       \
    V(2, a / 3)         \
    V(3, a / 5)         \
    V(4, -a / (a - 1))  \
    V(5, a + 5)         \
    V(6, a - 1)         \
    V(7, a* a)          \
    V(8, a / 12)        \
    V(9, a / (a - 1))   \
    V(10, a + 27)       \
    V(11, a - (a / 2))  \
    V(12, a* a - a)     \
    V(13, a / 7)        \
    V(14, a / (a - 3))  \
    V(15, a + 2)        \
    V(16, -a - 1)       \
    V(17, a& a)         \
    V(18, a & 12)       \
    V(19, a | (a - 1))  \
    V(20, a & 27)       \
    V(21, a + (a / 2))  \
    V(22, -a ^ a - a)   \
    V(23, a / -1)       \
    V(24, a / (-a - 3)) \
    V(25, a + 5)        \
    V(26, a - 1)        \
    V(27, a* a)         \
    V(28, a / 12)       \
    V(29, a / (a - 1))  \
    V(30, a + 27)       \
    V(31, a - (a / 2))  \
    V(32, a* a - a)     \
    V(33, a / 7)        \
    V(34, a / (a - 3))  \
    V(35, a + 2)        \
    V(36, -a - 1)       \
    V(37, a& a)         \
    V(38, a & 12)       \
    V(39, a | (a - 1))  \
    V(40, a & 27)       \
    V(41, a + (a / 2))  \
    V(42, -a ^ a - a)   \
    V(43, a / -1)       \
    V(44, a / (-a - 3)) \
    V(45, a + 5)        \
    V(46, a - 1)        \
    V(47, a* a)         \
    V(48, a / 12)       \
    V(49, a / (a - 1))  \
    V(50, a + 27)       \
    V(51, a - (a / 2))  \
    V(52, a* a - a)     \
    V(53, a / 7)        \
    V(54, a / (a - 3))  \
    V(55, a + 2)        \
    V(56, -a - 1)       \
    V(57, a& a)         \
    V(58, a & 12)       \
    V(59, a | (a - 1))  \
    V(60, a & 27)       \
    V(61, a + (a / 2))  \
    V(62, -a ^ a - a)   \
    V(63, a / -1)       \
    V(64, a / (-a - 3)) \
    V(65, a + 5)        \
    V(66, a - 1)        \
    V(67, a* a)         \
    V(68, a / 12)       \
    V(69, a / (a - 1))  \
    V(70, a + 27)       \
    V(71, a - (a / 2))  \
    V(72, a / a * a)    \
    V(73, a / 7)        \
    V(74, a / (a - 3))  \
    V(75, a + 2)        \
    V(76, -a - 1)       \
    V(77, a& a)         \
    V(78, a & 12)       \
    V(79, a | (a - 1))  \
    V(80, a & 27)       \
    V(81, a + (a / 2))  \
    V(82, -a ^ a - a)   \
    V(83, a / -1)       \
    V(84, a / (-a - 3)) \
    V(85, -a + 5)       \
    V(86, -a - 1)       \
    V(87, -a* a)        \
    V(88, -a / 12)      \
    V(89, -a / (a - 1)) \
    V(90, -a + 27)

class bitmap {
   private:
    unsigned int m_width;
    unsigned int m_height;
    std::vector<unsigned char> m_bitmap;

   public:
    bitmap(unsigned int width, unsigned int height)
        : m_bitmap(m_width * m_height, 0), m_width(width), m_height(height) {}

    void set_pixel(int x, int y, unsigned char value) {
        if (x >= 0 & y >= 0 & x < m_width & y < m_height) {
            m_bitmap[y * m_width + x] = value;
        }
    }

    unsigned char get_pixel(int x, int y) { return m_bitmap[y * m_width + x]; }

    unsigned int get_width() { return m_width; }

    unsigned int get_height() { return m_height; }
};

class object {
   protected:
    bool m_is_visible;
    unsigned int m_id;

    static unsigned int m_offset;

   public:
    object(unsigned int id) : m_id(id), m_is_visible(true) {}

    virtual unsigned int draw(bitmap& b) = 0;

    virtual std::string to_string() = 0;

    virtual unsigned int get_id() = 0;

    virtual int long_virtual_function(std::vector<int>& v,
                                      int start,
                                      int count) = 0;

    ATTRNOINLINE
    unsigned int get_id2() { return m_id; }

    ATTRNOINLINE
    bool is_visible() { return m_is_visible; }

    ATTRNOINLINE
    unsigned int get_id3() { return m_id + m_offset; };

    void show() { m_is_visible = true; }

    void hide() { m_is_visible = false; }
};

unsigned int object::m_offset = 1;

template <typename T>
T dist(T x, T y) {
    if (x > y) {
        return x - y;
    } else {
        return y - x;
    }
}

class point {
   public:
    unsigned int x;
    unsigned int y;

    point(unsigned int xx = 0, unsigned int yy = 0) : x(xx), y(yy) {}

    unsigned int distance(const point& other) {
        if (other.x == x) {
            return dist(x, other.x);
        } else if (other.y == y) {
            return dist(y, other.y);
        } else {
            unsigned int x_diff = dist(x, other.x);
            unsigned int y_diff = dist(y, other.y);
            return std::sqrt(x_diff * x_diff + y_diff * y_diff);
        }
    }

    std::string to_string() {
        char sbuf[200];
        sprintf(sbuf, "point(%d,%d)", x, y);
        return std::string(sbuf);
    }
};

class circle : virtual public object {
    point m_center;
    unsigned int m_diameter;

   public:
    circle(point center, int diameter)
        : object(1), m_center(center), m_diameter(diameter) {}

    unsigned int draw(bitmap& b) override {
        /*        (x - h)2 + (y - k)2 = r2
                (y - k)2 = r2 - (x - h2)
                (y - k) = std::sqrt(r2 - (x - h)2)
        */
        unsigned int r_pow_2 = m_diameter * m_diameter;

        for (unsigned int x = m_center.x - m_diameter;
             x < m_center.x + m_diameter; x++) {
            unsigned int x_diff = (x - m_center.x) * (x - m_center.x);
            unsigned int y_diff = std::sqrt(r_pow_2 - x_diff);
            b.set_pixel(x, y_diff + m_center.y, 255);
            b.set_pixel(x, -y_diff + m_center.y, 255);
        }

        return 4 * m_diameter - 2;
    }

    ATTRNOINLINE
    unsigned int get_id() override { return m_id; }

    std::string to_string() override {
        char sbuf[200];
        sprintf(sbuf, "circle(%s, diameter = %d)", m_center.to_string().c_str(),
                m_diameter);
        return std::string(sbuf);
    }

    int long_virtual_function(std::vector<int>& v,
                              int start,
                              int count) override {
        int sum = 0;
        for (int i = start; i < start + count; i++) {
            int a = v[i];

            if (a == 0) {
                sum += a;
            }
#define V(num, expr)                \
    else if (a == num) {            \
        sum += (expr) - (expr) / 2; \
    }
            VARIABLES
#undef V
        }
        return sum;
    }
};

class rectangle : virtual public object {
    point m_top_left;
    point m_bottom_right;

   public:
    rectangle(point p1, point p2) : object(2) {
        if (p1.x < p2.x) {
            if (p1.y < p2.y) {
                m_top_left = p1;
                m_bottom_right = p2;
            } else {
                m_top_left.x = p1.x;
                m_bottom_right.x = p2.x;
                m_top_left.y = p2.y;
                m_bottom_right.y = p1.y;
            }
        } else {
            if (p1.y < p2.y) {
                m_top_left.x = p2.x;
                m_bottom_right.x = p1.x;
                m_top_left.y = p1.y;
                m_bottom_right.y = p2.y;
            } else {
                m_top_left = p2;
                m_bottom_right = p1;
            }
        }
    }

    unsigned int draw(bitmap& b) override {
        for (int i = m_top_left.x; i < m_bottom_right.x; i++) {
            b.set_pixel(i, m_top_left.y, 255);
            b.set_pixel(i, m_bottom_right.y, 255);
        }

        for (int i = m_top_left.y; i < m_bottom_right.y; i++) {
            b.set_pixel(m_top_left.x, i, 255);
            b.set_pixel(m_bottom_right.x, i, 255);
        }

        return 2 * (m_bottom_right.y - m_top_left.y) +
               2 * (m_bottom_right.x - m_top_left.y);
    }

    ATTRNOINLINE
    unsigned int get_id() override { return m_id; }

    std::string to_string() override {
        char sbuf[200];
        sprintf(sbuf, "rectangle(%s, %s)", m_top_left.to_string().c_str(),
                m_bottom_right.to_string().c_str());
        return std::string(sbuf);
    }

    int long_virtual_function(std::vector<int>& v,
                              int start,
                              int count) override {
        int sum = 0;
        for (int i = start; i < start + count; i++) {
            int a = v[i];

            if (a == 0) {
                sum += a;
            }
#define V(num, expr)                \
    else if (a == num) {            \
        sum += (expr) * (expr) / 2; \
    }
            VARIABLES
#undef V
        }
        return sum;
    }
};

template <typename T>
void swap_if_larger(T& t1, T& t2) {
    if (t1 > t2) {
        std::swap(t1, t2);
    }
}

class line : virtual public object {
    point p1;
    point p2;

   public:
    line(point pp1, point pp2) : object(3), p1(pp1), p2(pp2) {}

    unsigned int draw(bitmap& b) override {
        if (p1.x == p2.x) {
            int y1 = p1.y;
            int y2 = p2.y;

            swap_if_larger(y1, y2);
            for (int i = y1; i <= y2; i++) {
                b.set_pixel(p1.x, i, 255);
            }

            return y2 - y1;
        } else if (p1.y == p2.y) {
            int x1 = p1.x;
            int x2 = p2.x;

            swap_if_larger(x1, x2);
            for (int i = x1; i <= x2; i++) {
                b.set_pixel(i, p1.y, 255);
            }

            return x2 - x1;
        } else {
            float fy1 = p1.y, fy2 = p2.y, fx1 = p1.x, fx2 = p2.x;

            float k = (fy1 - fy2) / (fx1 - fx2);
            float n = fy1 - k * fx1;

            swap_if_larger(fx1, fx2);

            for (unsigned int i = fx1; i <= fx2; i++) {
                unsigned int y = k * i + n;
                b.set_pixel(i, y, 255);
            }

            return fx2 - fx1;
        }
    }

    ATTRNOINLINE
    unsigned int get_id() override { return m_id; }

    std::string to_string() override {
        char sbuf[200];
        sprintf(sbuf, "line(%s, %s)", p1.to_string().c_str(),
                p2.to_string().c_str());
        return std::string(sbuf);
    }

    int long_virtual_function(std::vector<int>& v,
                              int start,
                              int count) override {
        int sum = 0;
        for (int i = start; i < start + count; i++) {
            int a = v[i];

            if (a == 0) {
                sum += a;
            }
#define V(num, expr)                        \
    else if (a == num) {                    \
        sum += (expr - 1) + (expr - 1) / 2; \
    }
            VARIABLES
#undef V
        }
        return sum;
    }
};

class monster : public line, public circle {
   public:
    monster()
        : object(4),
          line(point(0, 0), point(10, 8)),
          circle(point(20, 20), 0) {}

    unsigned int draw(bitmap& b) override { return 0; }

    ATTRNOINLINE
    unsigned int get_id() override { return m_id; }

    std::string to_string() override { return std::string("monster"); }

    int long_virtual_function(std::vector<int>& v,
                              int start,
                              int count) override {
        int sum = 0;
        for (int i = start; i < start + count; i++) {
            int a = v[i];

            if (a == 0) {
                sum += a;
            }
#define V(num, expr)                \
    else if (a == num) {            \
        sum += (expr) + (expr) / 2; \
    }
            VARIABLES
#undef V
        }
        return sum;
    }
};
