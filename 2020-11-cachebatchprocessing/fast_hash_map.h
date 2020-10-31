#include <cassert>
#include <functional>

template <typename T>
class hash_map_entry {
   public:
    hash_map_entry() : m_next_vector(nullptr) {}
    bool find(const T& value) {
        if (m_next_vector >= oneptr()) {
            if (*get_first() == value) {
                return true;
            }

            return std::find(m_next_vector->begin(), m_next_vector->end(),
                             value) != m_next_vector->end();
        } else {
            return false;
        }
    }

    void prefetch() { __builtin_prefetch(&m_value); }

    bool insert(const T& value) {
        if (find(value)) {
            return false;
        }

        if (m_next_vector == nullptr) {
            ::new (m_value) T(value);
            m_next_vector = oneptr();
        } else if (m_next_vector == oneptr()) {
            m_next_vector = new std::vector<T>();
            m_next_vector->push_back(value);
        } else {
            m_next_vector->push_back(value);
        }

        return true;
    }

    bool remove(const T& value) {
        bool result;

        if (m_next_vector >= oneptr()) {
            if (*get_first() == value) {
                if (m_next_vector == oneptr()) {
                    get_first()->~T();
                    m_next_vector = 0;
                } else {
                    std::swap(*get_first(), m_next_vector->back());
                    if (m_next_vector->size() == 1) {
                        delete m_next_vector;
                        m_next_vector = oneptr();
                    } else {
                        m_next_vector->pop_back();
                    }
                }

                result = true;
            } else {
                if (m_next_vector == oneptr()) {
                    result = false;
                } else {
                    auto it = std::find(m_next_vector->begin(),
                                        m_next_vector->end(), value);
                    if (it == m_next_vector->end()) {
                        result = false;
                    } else {
                        if (m_next_vector->size() > 1) {
                            std::iter_swap(it, m_next_vector->rbegin());
                            m_next_vector->pop_back();
                        } else {
                            delete m_next_vector;
                            m_next_vector = oneptr();
                        }
                        result = true;
                    }
                }
            }
        } else {
            result = false;
        }
        return result;
    }

    void dump(std::ostream& os) {
        if (m_next_vector >= oneptr()) {
            os << *get_first() << ", ";
            if (m_next_vector > oneptr()) {
                for (auto it = m_next_vector->begin();
                     it != m_next_vector->end(); ++it) {
                    os << *it << ", ";
                }
            }
        }
    }

   private:
    std::vector<T>* oneptr() const {
        return reinterpret_cast<std::vector<T>*>(1);
    }

    T* get_first() { return reinterpret_cast<T*>(m_value); }
    char m_value[sizeof(T)];
    std::vector<T>* m_next_vector;
};

template <typename T>
class simple_hash_map_entry {
   public:
    bool find(const T& value) {
        return std::find(m_values.begin(), m_values.end(), value) !=
               m_values.end();
    }

    bool insert(const T& value) {
        if (find(value)) {
            return false;
        }

        m_values.push_back(value);
        return true;
    }

    void prefetch() { __builtin_prefetch(&m_values); }

    bool remove(const T& value) {
        auto it = std::find(m_values.begin(), m_values.end(), value);
        if (it != m_values.end()) {
            std::iter_swap(it, m_values.rbegin());
            m_values.pop_back();
            return true;
        } else {
            return false;
        }
    }

   private:
    std::vector<T> m_values;
};

template <typename T, typename Q>
class fast_hash_map {
   public:
    fast_hash_map(size_t size) : m_values(size), m_size(size) {}

    ~fast_hash_map() {}

    bool insert(const T& value) {
        size_t entry = get_entry(value);

        return m_values[entry].insert(value);
    }

    bool remove(const T& value) {
        size_t entry = get_entry(value);

        return m_values[entry].remove(value);
    }

    bool find(const T& value) {
        size_t entry = get_entry(value);
        return m_values[entry].find(value);
    }

    std::vector<bool> find_multiple(const std::vector<T>& values) {
        std::vector<bool> result(values.size(), false);

        for (size_t i = 0; i < values.size(); i++) {
            result[i] = find(values[i]);
        }

        return result;
    }

    void dump(std::ostream& os) {
        for (size_t i = 0; i < m_values.size(); i++) {
            os << i << ": ";
            m_values[i].dump(os);
            os << std::endl;
        }
    }

    template <size_t look_ahead = 16>
    std::vector<bool> find_multiple_fast(const std::vector<T>& values) {
        std::vector<bool> result(values.size(), false);
        std::array<size_t, look_ahead> hashes;

        for (size_t i = 0; i < hashes.size(); i++) {
            hashes[i] = get_entry(values[i]);
        }

        for (size_t i = 0, j = look_ahead; i < values.size() - look_ahead;
             i++, j++) {
            size_t entry = hashes[i % hashes.size()];
            result[i] = m_values[entry].find(values[i]);

            entry = get_entry(values[j]);
            hashes[j % hashes.size()] = entry;
            m_values[entry].prefetch();
        }

        for (size_t i = values.size() - look_ahead; i < values.size(); i++) {
            result[i] = find(values[i]);
        }

        return result;
    }

    template <size_t look_ahead = 64>
    std::vector<bool> find_multiple_fast2(const std::vector<T>& values) {
        std::vector<bool> result(values.size(), false);
        std::array<size_t, look_ahead> hashes;
        size_t entry, index;

        size_t len = (values.size() / look_ahead) * look_ahead;

        for (size_t i = 0; i < len; i += look_ahead) {
            for (size_t j = i, k = 0; k < look_ahead; j++, k++) {
                entry = get_entry(values[j]);
                hashes[k] = entry;
                m_values[entry].prefetch();
            }

            for (size_t j = i, k = 0; k < look_ahead; j++, k++) {
                result[j] = m_values[hashes[k]].find(values[j]);
            }
        }

        for (size_t i = len; i < values.size(); i++) {
            result[i] = find(values[i]);
        }

        return result;
    }

    std::vector<Q> m_values;
    size_t m_size;
    std::hash<T> m_hash;

    size_t get_entry(const T& v) { return m_hash(v) % m_size; }
};
