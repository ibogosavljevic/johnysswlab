#include <cmath>

#define NOINLINE /*__attribute__((noinline))*/

class bitmap {
   private:
    unsigned int m_width;
    unsigned int m_height;
    std::vector<unsigned char> m_bitmap;

   public:
    bitmap(unsigned int width, unsigned int height)
        : m_bitmap(m_width * m_height, 0), m_width(width), m_height(height) {}

    void set_pixel(int x, int y, unsigned char value) {
        if (x >= 0 & y >= 0 & x < m_width & y < m_width) {
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

   public:
    object(unsigned int id) : m_id(id), m_is_visible(true) {}

    virtual unsigned int draw(bitmap& b) = 0;

    virtual std::string to_string() = 0;

    virtual unsigned int get_id() = 0;

    unsigned int get_id2() { return m_id; }

    bool is_visible() { return m_is_visible; }

    void show() { m_is_visible = true; }

    void hide() { m_is_visible = false; }
};

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

    NOINLINE
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

    NOINLINE
    unsigned int get_id() override { return m_id; }

    std::string to_string() override {
        char sbuf[200];
        sprintf(sbuf, "circle(%s, diameter = %d)", m_center.to_string().c_str(),
                m_diameter);
        return std::string(sbuf);
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

    NOINLINE
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

    NOINLINE
    unsigned int get_id() override { return m_id; }

    std::string to_string() override {
        char sbuf[200];
        sprintf(sbuf, "rectangle(%s, %s)", m_top_left.to_string().c_str(),
                m_bottom_right.to_string().c_str());
        return std::string(sbuf);
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

    NOINLINE
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

    NOINLINE
    unsigned int get_id() override { return m_id; }

    std::string to_string() override {
        char sbuf[200];
        sprintf(sbuf, "line(%s, %s)", p1.to_string().c_str(),
                p2.to_string().c_str());
        return std::string(sbuf);
    }
};

class monster : public line, public circle {
   public:
    monster()
        : object(4),
          line(point(0, 0), point(10, 8)),
          circle(point(20, 20), 0) {}

    NOINLINE
    unsigned int draw(bitmap& b) override { return 0; }

    NOINLINE
    unsigned int get_id() override { return m_id; }

    std::string to_string() override { return std::string("monster"); }
};
