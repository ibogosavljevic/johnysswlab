
template <typename T>
class fast_hash_map {
    fast_hash_map(size_t size)
        : m_values(size), m_value_used(size), m_size(size) {}

    void insert(const T& value) {
        size_t entry = get_entry(value);

        if (m_value_used[entry]) {
            member* m = new member();
            m->m_next = m_values[entry].m_next;
            m_value[entry].m_next = m;
            new (m->m_value) T(value);
        } else {
            m_value_used[entry] = true;
            new (m_values[entry].m_value) T(value);
        }
    }

   private:
    struct member {
        char m_value[sizeof(T)];
        member* m_next;

        member() : m_next(nullptr) {}
    };

    std::vector<bool> m_value_used;
    std::vector<member> m_values;
    size_t m_size;
    std::hash<T> m_hash;

    size_t get_entry(const T& v) { return m_hash(v) % m_size; }
};