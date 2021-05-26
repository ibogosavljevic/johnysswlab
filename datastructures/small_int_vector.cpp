#include <cstddef>
#include <cstdlib>
#include <iostream>

class small_int_vector {
   private:
    static constexpr int static_capacity = 4;
    static constexpr size_t heap_size_mask = 1ull << (sizeof(size_t) * 8 - 1);

    union data_t {
        struct {
            int* m_data;
            size_t m_capacity;
        } m_heap_data;

        int m_preallocated[static_capacity];
    };

    data_t m_data;
    size_t m_size;

   public:
    small_int_vector(size_t capacity) {
        if (capacity > static_capacity) {
            m_data.m_heap_data.m_capacity = capacity;
            m_data.m_heap_data.m_data = (int*)malloc(sizeof(int) * capacity);
            m_size = heap_size_mask;
        } else {
            m_size = 0;
        }
    }

    bool is_preallocated() { return (m_size & heap_size_mask) == 0; }

    int& operator[](size_t index) {
        if (is_preallocated()) {
            return m_data.m_preallocated[index];
        } else {
            return m_data.m_heap_data.m_data[index];
        }
    }

    size_t size() { return m_size & (~heap_size_mask); }
};

void print(small_int_vector& s) {
    for (int i = 0; i < s.size(); ++i) {
        std::cout << s[i] << ", ";
    }
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "Size of small_int_vector = " << sizeof(small_int_vector)
              << std::endl;

    small_int_vector v0(0), v1(1), v2(2), v3(3), v4(4), v5(5);

    v1[0] = 5;
    v2[0] = 5;
    v3[0] = 5;
    v4[0] = 5;
    v5[0] = 5;

    v2[1] = 4;
    v3[1] = 4;
    v4[1] = 4;
    v5[1] = 4;

    v3[2] = 3;
    v4[2] = 3;
    v5[2] = 3;

    v4[3] = 2;
    v5[3] = 2;

    v5[4] = 1;

    print(v1);
    print(v2);
    print(v3);
    print(v4);
    print(v5);
    return 0;
}