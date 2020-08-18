#include <cmath>

class bitmap {
};

class object {
protected:
    bool m_is_visible;
public:
    virtual void draw(const bitmap& b) = 0;

    virtual std::string to_string() = 0;

    bool is_visible() { return m_is_visible; }
};

class point {
    int x;
    int y;

    int distance(const point& other) {
        if (other.x == x) {
            return std::abs(x - other.x);
        } else if (other.y == y) {
            return std::abs(y - other.y);
        } else {
            int x_diff = std::abs(x - other.x);
            int y_diff = std::abs(y - other.y);
            return std::sqrt(x_diff * x_diff + y_diff * y_diff);
        }
    }
};

class circle : virtual public object {
    point m_center;
    int m_diameter;
public:

    void draw(const bitmap& b) override {

    }

    std::string to_string() {
        return std::string("Circle");
    }
};

class rectangle : virtual public object {
    point m_top_left;
    point m_bottom_right;
public:

    void draw(const bitmap& b) override {

    }

    std::string to_string() {
        return std::string("Rectangle");
    }
};

class line : virtual public object {
    point m_top_left;
    point m_bottom_right;
public:

    void draw(const bitmap& b) override {

    }

    std::string to_string() {
        return std::string("Line");
    }
};

class monster: public line, public circle {
public:
    void draw(const bitmap& b) override {

    }

    std::string to_string() {
        return std::string("Monster");
    }
};
