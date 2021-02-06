template <typename Parent, typename... Children>
class polymorphic_vector {
   private:
    template <typename Base, typename... Others>
    class alignas(16) polymorphic {
       private:
        static constexpr size_t round_to_closest_16(size_t size) {
            return ((size % 16) == 0) ? size : ((size / 16) + 1) * 16;
        }
        template <typename T>
        static constexpr size_t get_max_type_size() {
            return sizeof(T);
        }

        template <typename T, typename Arg, typename... Args>
        static constexpr size_t get_max_type_size() {
            return max(sizeof(T), get_max_type_size<Arg, Args...>());
        }

        static constexpr size_t max(size_t v1, size_t v2) {
            return v1 > v2 ? v1 : v2;
        }

        class wrapper {
           public:
            static constexpr int m_size = get_max_type_size<Others...>();
            char m_data[m_size];
        };

       public:
        wrapper m_wrapper;
    };

    using pointer_diff_t = int16_t;

    std::vector<polymorphic<Parent, Children...>> m_vector;
    std::vector<pointer_diff_t> m_pointer_diff;

    template <typename BaseAddr, typename ModifiedAddr>
    pointer_diff_t get_pointer_diff(BaseAddr base, ModifiedAddr modified) {
        char* base_p = reinterpret_cast<char*>(base);
        char* modified_p = reinterpret_cast<char*>(modified);
        return base_p - modified_p;
    }

    template <typename BaseAddr, typename ModifiedAddr>
    ModifiedAddr get_modified_addr(BaseAddr base, pointer_diff_t diff) {
        char* base_p = static_cast<char*>(base);
        return reinterpret_cast<ModifiedAddr>(base_p - diff);
    }

   public:
    polymorphic_vector(int size) : m_vector(size), m_pointer_diff(size) {}
    polymorphic_vector() : m_vector(), m_pointer_diff() {}

    Parent* get(int index) {
        return get_modified_addr<char*, Parent*>(
            m_vector[index].m_wrapper.m_data, m_pointer_diff[index]);
    }

    template <typename Q>
    void push_back(const Q& q) {
        static_assert(sizeof(Q) <= sizeof(polymorphic<Parent, Children...>),
                      "Size of object we are putting must fit");
        static_assert(std::is_base_of<Parent, Q>::value,
                      "The type must be derived from the parent");

        m_vector.emplace_back();
        ::new (m_vector.back().m_wrapper.m_data) Q(q);
        m_pointer_diff.emplace_back(get_pointer_diff(
            m_vector.back().m_wrapper.m_data,
            static_cast<Parent*>(
                reinterpret_cast<Q*>(m_vector.back().m_wrapper.m_data))));
    }

    template <typename Q, typename... Args>
    void emplace_back(const Args&... args) {
        static_assert(sizeof(Q) <= sizeof(polymorphic<Parent, Children...>),
                      "Size of object we are putting must fit");
        static_assert(std::is_base_of<Parent, Q>::value,
                      "The type must be derived from the parent");

        m_vector.emplace_back();
        ::new (m_vector.back().m_wrapper.m_data) Q(args...);
        m_pointer_diff.emplace_back(get_pointer_diff(
            m_vector.back().m_wrapper.m_data,
            static_cast<Parent*>(
                reinterpret_cast<Q*>(m_vector.back().m_wrapper.m_data))));
    }

    void shuffle() {
        std::vector<int> indexes(m_vector.size());

        std::iota(indexes.begin(), indexes.end(), 0);

        for (int i = 0; i < m_vector.size(); i++) {
            std::swap(m_pointer_diff[i], m_pointer_diff[indexes[i]]);
            std::swap(m_vector[i], m_vector[indexes[i]]);
        }
    }

    void reserve(int size) {
        m_vector.reserve(size);
        m_pointer_diff.reserve(size);
    }

    int size() { return m_vector.size(); }
};
