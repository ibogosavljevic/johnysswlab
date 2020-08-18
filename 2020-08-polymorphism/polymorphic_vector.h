constexpr int round_to_closest_16(int size)  {
    return ((size % 16) == 0) ? size : ((size / 16) + 1) * 16;
}


template <typename T, int Size>
class polymorphic_vector {
private:
    class alignas(16) wrapper {
    public:
        static constexpr int m_size = round_to_closest_16(Size);
        char m_data[m_size];
    };

    std::vector<wrapper> m_vector;

public:
    polymorphic_vector(int size) : m_vector(size) {}
    polymorphic_vector() : m_vector() {}

    T* get(int index) {
        return reinterpret_cast<T*>(&(m_vector[index].m_data));
    }

    template <typename Q>
    void push_back(const Q& q) {
        static_assert(sizeof(Q) <= wrapper::m_size);
        static_assert(std::is_base_of<T, Q>::value);

        wrapper w;
        
        std::memcpy(&w.m_data, &q, sizeof(Q));
        m_vector.push_back(std::move(w));
    }

    template <typename Q, typename ...Args>
    void emplace_back(const Args &... args) {
        static_assert(sizeof(Q) <= wrapper::m_size);
        static_assert(std::is_base_of<T, Q>::value);

        m_vector.emplace_back();
        ::new (&m_vector.back()) Q(args...);
    }

    void shuffle() {
        std::random_shuffle(m_vector.begin(), m_vector.end());
    }

    void reserve(int size) {
        m_vector.reserve(size);
    }

};
